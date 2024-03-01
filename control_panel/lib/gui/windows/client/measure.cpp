#include <thread>
#include <cstdint>

#include "imgui.h"
#include "imgui_internal.h"
#include <nfd.hpp>
#include <sys/types.h>
#include <utf/utf.hpp>

#include "json/conversion.hpp"
#include "ad5933/calibration/calibration.hpp"
#include "imgui_custom/spinner.hpp"
#include "imgui_custom/input_items.hpp"
#include "imgui_custom/char_filters.hpp"
#include "magic/misc/gettimeofday.hpp"
#include "gui/boilerplate.hpp"

#include "gui/windows/client/measure.hpp"

namespace GUI {
    namespace Windows {
        Measure::Measure(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) :
            index { index },
            shm { shm }
        {
            name.append(utf::as_u8(std::to_string(index)));
        }
        
        Measure::~Measure() {
            stop_source.request_stop();
        }

        const std::optional<Lock> Measure::draw_inner() {
            if(status == Status::NotLoaded || status == Status::MeasuringSingle || status == Status::MeasuringPeriodic) {
                ImGui::BeginDisabled();
                draw_input_elements();
                ImGui::EndDisabled();

                if(status == Status::MeasuringSingle || status == Status::MeasuringPeriodic) {
                    ImGui::BeginDisabled();
                    ImGui::Button("Load");
                    ImGui::EndDisabled();
                } else {
                    if(ImGui::Button("Load")) {
                        if(load()) {
                            status = Status::Loaded;
                        }
                    }
                }

                ImGui::BeginDisabled();

                ImGui::Button("Single");
                if(status == Status::MeasuringSingle) {
                    ImGui::EndDisabled();

                    ImGui::SameLine();
                    ImGui::ProgressBar(progress_bar_fraction);

                    ImGui::BeginDisabled();
                }
                ImGui::Button("Periodic");
                if(status == Status::MeasuringPeriodic) {
                    ImGui::EndDisabled();

                    ImGui::SameLine();
                    ImGui::ProgressBar(progress_bar_fraction);

                    ImGui::BeginDisabled();
                }

                ImGui::EndDisabled();

                if(periodically_sweeping) {
                    if(ImGui::Button("Stop")) {
                        stop();
                    }
                }
            } else {
                draw_input_elements();
                if(ImGui::Button("Load")) {
                    if(load()) {
                        status = Status::Loaded;
                    }
                }
                if(ImGui::Button("Single")) {
                    single();
                }
                if(ImGui::Button("Periodic")) {
                    periodic();
                }
            }

            if(status == Status::MeasuringSingle || status == Status::MeasuringPeriodic) {
                return Lock::Measure; 
            } else {
                return std::nullopt;
            }
        }

        void Measure::draw(bool &enable, const ImGuiID side_id, Lock& lock) {
            if(first) {
                ImGui::DockBuilderDockWindow((const char*) name.c_str(), side_id);
                first = false;
            }

            if(ImGui::Begin((const char*) name.c_str(), &enable, ImGuiWindowFlags_NoMove) == false) {
                ImGui::End();
                return;
            }

            if(lock != Lock::Released && lock != Lock::Measure) {
                ImGui::BeginDisabled();
                draw_inner();
                ImGui::EndDisabled();
            } else {
                const auto ret_lock { draw_inner() };
                if(lock == Lock::Measure && ret_lock.has_value() == false) {
                    lock = Lock::Released;
                } else if(ret_lock.has_value()) {
                    lock = ret_lock.value();
                }
            }

            ImGui::End();
        }

        bool Measure::load() {
            nfdchar_t *outPath = nullptr;
            const std::array<nfdfilteritem_t, 1> filterItem { { "Calibration", "json" } };
            switch(NFD::OpenDialog(outPath, filterItem.data(), static_cast<nfdfiltersize_t>(filterItem.size()))) {
                case NFD_CANCEL:
                    std::printf("GUI::Windows::Measure::load(): NFD_CANCEL: User pressed cancel!\n");
                    if(outPath != nullptr) {
                        NFD::FreePath(outPath);
                        outPath = nullptr;
                    }
                    return false;
                case NFD_ERROR:
                    std::printf("ERROR: GUI::Windows::Measure::load(): NFD_ERROR: %s\n", NFD::GetError());
                    if(outPath != nullptr) {
                        NFD::FreePath(outPath);
                        outPath = nullptr;
                    }
                    return false;
                default:
                    try {
                        json j;
                        std::ifstream(outPath) >> j;
                        if(outPath != nullptr) {
                            NFD::FreePath(outPath);
                            outPath = nullptr;
                        }
                        const ns::CalibrationFile calibration_file = j;
                        const auto calibration_pair = calibration_file.unwrap();
                        configs.calibration = calibration_pair.first;
                        configs.measurement = calibration_pair.first;
                        calibration_vectors.calibration = calibration_pair.second;
                        calibration_vectors.freq_uint32_t = configs.calibration.get_freq_vector<uint32_t>();
                        calibration_vectors.freq_float = configs.calibration.get_freq_vector<float>();
                        inputs.numeric.freq_start = configs.calibration.get_start_freq().unwrap();
                        inputs.numeric.freq_end = configs.calibration.get_freq_end();
                        return true;
                    } catch(const std::exception& e) {
                        std::cout << "ERROR: GUI::Windows::Measure::load(): exception: " << e.what() << std::endl;
                        if(outPath != nullptr) {
                            NFD::FreePath(outPath);
                            outPath = nullptr;
                        }
                        return false;
                    }
            }
        }

        void Measure::stop() {
            periodically_sweeping = false;
        }

        void Measure::single_cb(std::stop_token st, Measure& self) {
            const auto freq_start_it { std::find(self.calibration_vectors.freq_float.begin(), self.calibration_vectors.freq_float.end(), static_cast<float>(self.configs.measurement.get_start_freq().unwrap())) };
            if(freq_start_it == self.calibration_vectors.freq_float.end()) {
                self.status = Status::Failed;
                return;
            }

            const auto freq_end_it { std::find(self.calibration_vectors.freq_float.begin(), self.calibration_vectors.freq_float.end(), static_cast<float>(self.configs.measurement.get_freq_end())) };
            if(freq_end_it == self.calibration_vectors.freq_float.end()) {
                self.status = Status::Failed;
                return;
            }

            const std::vector<float> tmp_freq(freq_start_it, freq_end_it + 1);

            const size_t freq_start_distance = std::distance(self.calibration_vectors.freq_float.begin(), freq_start_it);
            const size_t freq_end_distance = std::distance(self.calibration_vectors.freq_float.begin(), freq_end_it);
            const auto cal_start_it = self.calibration_vectors.calibration.begin() + freq_start_distance;
            const auto cal_end_it = self.calibration_vectors.calibration.begin() + freq_end_distance;

            const std::vector<AD5933::Calibration<float>> tmp_calibration(cal_start_it, cal_end_it + 1);

            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Events::Commands::Sweep::Configure{
                        self.configs.measurement.to_raw_array()
                    }
                }
            );

            const uint16_t wished_size = self.configs.measurement.get_num_of_inc().unwrap() + 1;
            assert(wished_size <= self.calibration_vectors.calibration.size());
            const float progress_bar_step = 1.0f / static_cast<float>(wished_size);
            const float timeout_ms =
                (1.0f / static_cast<float>(self.configs.measurement.get_start_freq().unwrap())) 
                * static_cast<float>(self.configs.measurement.get_settling_time_cycles_number().unwrap())
                * AD5933::Masks::Or::SettlingTimeCyclesHB::get_multiplier_float(self.configs.measurement.get_settling_time_cycles_multiplier())
                * 1'000'000.0f
                * 10.0f;
            const auto boost_timeout_ms { boost::posix_time::milliseconds(static_cast<size_t>(timeout_ms)) };

            bool first_iteration { true };
            do {
                self.progress_bar_fraction = 0.0f;
                std::vector<AD5933::Data> tmp_raw_measurement;
                tmp_raw_measurement.reserve(wished_size);
                std::vector<AD5933::Measurement<float>> tmp_measurement;
                tmp_measurement.reserve(wished_size);

                self.shm->cmd.send(
                    BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                        self.index,
                        Magic::Events::Commands::Sweep::Run{}
                    }
                );
                do {
                    const auto rx_payload { self.shm->active_devices[self.index].information->read_for(boost_timeout_ms) };
                    if(rx_payload.has_value() == false) {
                        self.status = Status::Failed;
                        std::cout << "ERROR: GUI::Windows::Measure::single_cb: sweep: timeout\n";
                        return;
                    }

                    if(st.stop_requested()) {
                        self.status = Status::Failed;
                        std::cout << "INFO: GUI::Windows::Measure::single_cb: stopping measurement\n";
                        return;
                    }

                    bool is_valid_data = false;
                    std::visit([&is_valid_data, &tmp_raw_measurement, &tmp_measurement, &tmp_calibration](auto&& event) {
                        using T_Decay = std::decay_t<decltype(event)>;
                        if constexpr(std::is_same_v<T_Decay, Magic::Events::Results::Sweep::ValidData>) {
                            std::array<uint8_t, 4> raw_array;
                            std::copy(event.real_imag_registers_data.begin(), event.real_imag_registers_data.end(), raw_array.begin());

                            const AD5933::Data tmp_data { raw_array };
                            tmp_raw_measurement.push_back(tmp_data);

                            const AD5933::Measurement<float> tmp_meas { tmp_data, tmp_calibration[tmp_measurement.size()] };
                            tmp_measurement.push_back(tmp_meas);

                            is_valid_data = true;
                        }
                    }, rx_payload.value());

                    if(is_valid_data == false) {
                        self.status = Status::Failed;
                        std::cout << "ERROR: GUI::Windows::Measure::single_cb: measure received wrong event result packet\n";
                        return;
                    }
                    self.progress_bar_fraction += progress_bar_step;
                } while(tmp_measurement.size() != wished_size);

                self.single_vectors.raw_measurement = tmp_raw_measurement;
                self.single_vectors.measurement = tmp_measurement;
                if(first_iteration) {
                    self.single_vectors.freq_float = tmp_freq;
                    first_iteration = false;
                }
            } while(self.periodically_sweeping);

            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Events::Commands::Sweep::End{}
                }
            );
            self.status = Status::Loaded;
            self.single_plotted = false;
        }

        void Measure::single() {
            status = Status::MeasuringSingle;
            std::jthread t1(single_cb, std::ref(*this));
            stop_source = t1.get_stop_source();
            t1.detach();
        }

        void Measure::periodic_cb(std::stop_token st, Measure& self) {
            const auto freq_start_it { std::find(self.calibration_vectors.freq_float.begin(), self.calibration_vectors.freq_float.end(), static_cast<float>(self.configs.measurement.get_start_freq().unwrap())) };
            if(freq_start_it == self.calibration_vectors.freq_float.end()) {
                self.status = Status::Failed;
                return;
            }

            const auto freq_end_it { std::find(self.calibration_vectors.freq_float.begin(), self.calibration_vectors.freq_float.end(), static_cast<float>(self.configs.measurement.get_freq_end())) };
            if(freq_end_it == self.calibration_vectors.freq_float.end()) {
                self.status = Status::Failed;
                return;
            }

            const std::vector<float> tmp_freq(freq_start_it, freq_end_it + 1);

            const size_t freq_start_distance = std::distance(self.calibration_vectors.freq_float.begin(), freq_start_it);
            const size_t freq_end_distance = std::distance(self.calibration_vectors.freq_float.begin(), freq_end_it);
            const auto cal_start_it = self.calibration_vectors.calibration.begin() + freq_start_distance;
            const auto cal_end_it = self.calibration_vectors.calibration.begin() + freq_end_distance;

            const std::vector<AD5933::Calibration<float>> tmp_calibration(cal_start_it, cal_end_it + 1);

            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Events::Commands::Sweep::Configure{
                        self.configs.measurement.to_raw_array()
                    }
                }
            );

            const uint16_t wished_size = self.configs.measurement.get_num_of_inc().unwrap() + 1;
            assert(wished_size <= self.calibration_vectors.calibration.size());
            const float progress_bar_step = 1.0f / static_cast<float>(wished_size);
            const float timeout_ms =
                (1.0f / static_cast<float>(self.configs.measurement.get_start_freq().unwrap())) 
                * static_cast<float>(self.configs.measurement.get_settling_time_cycles_number().unwrap())
                * AD5933::Masks::Or::SettlingTimeCyclesHB::get_multiplier_float(self.configs.measurement.get_settling_time_cycles_multiplier())
                * 1'000'000.0f
                * 10.0f;
            const auto boost_timeout_ms { boost::posix_time::milliseconds(static_cast<size_t>(timeout_ms)) };

            bool first_iteration { true };
            do {
                self.progress_bar_fraction = 0.0f;
                std::vector<AD5933::Data> tmp_raw_measurement;
                tmp_raw_measurement.reserve(wished_size);
                std::vector<AD5933::Measurement<float>> tmp_measurement;
                tmp_measurement.reserve(wished_size);

                self.shm->cmd.send(
                    BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                        self.index,
                        Magic::Events::Commands::Sweep::Run{}
                    }
                );
                do {
                    const auto rx_payload { self.shm->active_devices[self.index].information->read_for(boost_timeout_ms) };
                    if(rx_payload.has_value() == false) {
                        self.status = Status::Failed;
                        std::cout << "ERROR: GUI::Windows::Measure::single_cb: sweep: timeout\n";
                        return;
                    }

                    if(st.stop_requested()) {
                        self.status = Status::Failed;
                        std::cout << "INFO: GUI::Windows::Measure::single_cb: stopping measurement\n";
                        return;
                    }

                    bool is_valid_data = false;
                    std::visit([&is_valid_data, &tmp_raw_measurement, &tmp_measurement, &tmp_calibration](auto&& event) {
                        using T_Decay = std::decay_t<decltype(event)>;
                        if constexpr(std::is_same_v<T_Decay, Magic::Events::Results::Sweep::ValidData>) {
                            std::array<uint8_t, 4> raw_array;
                            std::copy(event.real_imag_registers_data.begin(), event.real_imag_registers_data.end(), raw_array.begin());

                            const AD5933::Data tmp_data { raw_array };
                            tmp_raw_measurement.push_back(tmp_data);

                            const AD5933::Measurement<float> tmp_meas { tmp_data, tmp_calibration[tmp_measurement.size()] };
                            tmp_measurement.push_back(tmp_meas);

                            is_valid_data = true;
                        }
                    }, rx_payload.value());

                    if(is_valid_data == false) {
                        self.status = Status::Failed;
                        std::cout << "ERROR: GUI::Windows::Measure::single_cb: measure received wrong event result packet\n";
                        return;
                    }
                    self.progress_bar_fraction += progress_bar_step;
                } while(tmp_measurement.size() != wished_size);
                
                mytimeval64_t tv;
                gettimeofday(&tv, nullptr);
                const double seconds = static_cast<double>(tv.tv_sec);
                const double useconds_to_seconds = static_cast<double>(tv.tv_usec) / 1000000.0;

                const PeriodicPoint tmp_periodic_point {
                    .time_point = seconds + useconds_to_seconds,
                    .raw_measurement = tmp_raw_measurement,
                    .measurement = tmp_measurement,
                };

                self.periodic_vectors.periodic_points.push(tmp_periodic_point);

                if(first_iteration) {
                    self.periodic_vectors.freq_float = tmp_freq;
                    first_iteration = false;
                }
            } while(self.periodically_sweeping);

            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Events::Commands::Sweep::End{}
                }
            );
            self.status = Status::Loaded;
            self.single_plotted = false;
        }

        void Measure::periodic() {
            status = Status::MeasuringPeriodic;
            periodically_sweeping = true;
            std::jthread t1(periodic_cb, std::ref(*this));
            stop_source = t1.get_stop_source();
            t1.detach();
        }
        
        void Measure::draw_input_elements() {
            if(ImGui::Input_uint32_t_WithCallbackTextReadOnly(
                "Start Frequency [Hz]",
                &inputs.numeric.freq_start,
                configs.calibration.get_inc_freq().unwrap(),
                configs.calibration.get_inc_freq().unwrap(),
                ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter,
                ImGui::plus_minus_dot_char_filter_cb
            )) {
                if(inputs.numeric.freq_start < configs.calibration.get_start_freq().unwrap()) {
                    inputs.numeric.freq_start = configs.calibration.get_start_freq().unwrap();
                } else if(inputs.numeric.freq_start >= inputs.numeric.freq_end) {
                    inputs.numeric.freq_start -= configs.calibration.get_inc_freq().unwrap();
                } else {
                    configs.measurement.set_start_freq(AD5933::uint_startfreq_t { inputs.numeric.freq_start });
                    configs.measurement.set_num_of_inc(AD5933::uint9_t { static_cast<uint16_t>((inputs.numeric.freq_end - inputs.numeric.freq_start) / configs.calibration.get_inc_freq().unwrap()) });
                }
            }

            if(ImGui::Input_uint32_t_WithCallbackTextReadOnly(
                "End Frequency [Hz]",
                &inputs.numeric.freq_end,
                configs.calibration.get_inc_freq().unwrap(),
                configs.calibration.get_inc_freq().unwrap(),
                ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter,
                ImGui::plus_minus_dot_char_filter_cb
            )) {
                if(inputs.numeric.freq_end > configs.calibration.get_freq_end()) {
                    inputs.numeric.freq_end = configs.calibration.get_freq_end();
                } else if (inputs.numeric.freq_end <= inputs.numeric.freq_start) {
                    inputs.numeric.freq_end += configs.calibration.get_inc_freq().unwrap();
                } else {
                    configs.measurement.set_num_of_inc(AD5933::uint9_t { static_cast<uint16_t>((inputs.numeric.freq_end - inputs.numeric.freq_start) / configs.calibration.get_inc_freq().unwrap())  });
                }
            }

            if(ImGui::Combo(
                "Settling Time Cycles Multiplier",
                &inputs.gui.settling_multiplier_combo,
                "x1\0""x2\0""x4\0"
            )) {
                configs.measurement.set_settling_time_cycles_multiplier(
                    AD5933::Masks::Or::SettlingTimeCyclesHB::Multiplier(
                        inputs.gui.settling_multiplier_combo
                    )
                );
            }

            if(ImGui::SliderInt("Settling Time Cycles Number", &inputs.numeric.settling_number, 1, 511)) {
                configs.measurement.set_settling_time_cycles_number(AD5933::uint9_t { static_cast<uint16_t>(inputs.numeric.settling_number) });
            }
        }
    }
}