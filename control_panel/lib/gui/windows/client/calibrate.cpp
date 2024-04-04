#include <thread>
#include <fstream>

#include <nfd.hpp>
#include <utf/utf.hpp>

#include "imgui_internal.h"

#include "gui/boilerplate.hpp"
#include "ad5933/masks/maps.hpp"
#include "imgui_custom/input_items.hpp"

#include "gui/windows/client/calibrate.hpp"
#include <trielo/trielo.hpp>

namespace GUI {
    namespace Windows {
        Calibrate::Calibrate(const size_t index, std::shared_ptr<BLE_Client::SHM::Parent> shm) :
            index { index },
            shm{ shm }
        {
            name.append(utf::as_u8(std::to_string(index)));
        }
        
        Calibrate::~Calibrate() {
            stop_source.request_stop();
        }

        void Calibrate::draw_input_fields() {
            if(ImGui::Slider_uint32_t_Valid(
                "Start Frequency [Hz]",
                &fields.freq_start,
                Min::freq_start,
                Max::freq_start,
                true,
                "%u",
                ImGuiSliderFlags_AlwaysClamp
            )) {
                config.set_start_freq(AD5933::uint_startfreq_t{ fields.freq_start });
                max.freq_inc = Max::freq - fields.freq_start;
                if(fields.freq_inc > max.freq_inc) {
                    fields.freq_inc = max.freq_inc;
                    config.set_inc_freq(AD5933::uint_incfreq_t{ fields.freq_inc });
                }
                max.num_of_inc = ((Max::freq - fields.freq_start) / fields.freq_inc) > Max::nine_bit ? Max::nine_bit : ((Max::freq - fields.freq_start) / fields.freq_inc);
                if(fields.num_of_inc > max.num_of_inc) {
                    fields.num_of_inc = max.num_of_inc;
                    config.set_num_of_inc(AD5933::uint9_t{ fields.num_of_inc });
                }
            }

            if(ImGui::Slider_uint32_t_Valid(
                "Increment Frequency [Hz]",
                &fields.freq_inc,
                Min::freq_inc,
                max.freq_inc,
                true,
                "%u",
                ImGuiSliderFlags_AlwaysClamp
            )) {
                config.set_inc_freq(AD5933::uint_incfreq_t{ fields.freq_inc });
                max.num_of_inc = ((Max::freq - fields.freq_start) / fields.freq_inc) > Max::nine_bit ? Max::nine_bit : ((Max::freq - fields.freq_start) / fields.freq_inc);
                if(fields.num_of_inc > max.num_of_inc) {
                    fields.num_of_inc = max.num_of_inc;
                    config.set_num_of_inc(AD5933::uint9_t{ fields.num_of_inc });
                }
            }

            if(ImGui::Slider_uint16_t_Valid(
                "Number of Increments",
                &fields.num_of_inc,
                Min::num_of_inc,
                max.num_of_inc,
                true,
                "%u",
                ImGuiSliderFlags_AlwaysClamp
            )) {
                config.set_num_of_inc(AD5933::uint9_t{ fields.num_of_inc });
            }

            ImGui::Separator(); 

            ImGui::SliderFloat(
                "Calibration Impedance [Ohm]",
                &fields.impedance,
                std::numeric_limits<float>::min(),
                1'000'000.0f,
                "%.3f",
                ImGuiSliderFlags_AlwaysClamp
            );

            ImGui::Separator(); 

            if(ImGui::Combo(
                "Output Excitation Voltage Range",
                &fields.voltage_range_combo,
                "2 Vppk\0""1 Vppk\0""400 mVppk\0""200 mVppk\0"
            )) {
                config.set_voltage_range(std::next(AD5933::Masks::Or::Ctrl::HB::voltage_map.begin(), fields.voltage_range_combo)->first);
            }

            if(ImGui::Combo("PGA Gain", &fields.pga_gain_combo, "x1\0""x5\0")) {
                config.set_PGA_gain(std::next(AD5933::Masks::Or::Ctrl::HB::pga_gain_map.begin(), fields.pga_gain_combo)->first);
            }

            ImGui::Separator(); 

            if(ImGui::SliderInt("Settling Time Cycles", &fields.settling_number, Min::settling_cycles, max.settling_cycles, "%d", ImGuiSliderFlags_AlwaysClamp)) {
                config.set_settling_time_cycles_number(AD5933::uint9_t{ static_cast<uint16_t>(fields.settling_number) });
            }

            if(ImGui::Combo(
                "Settling Time Cycles Multiplier",
                &fields.settling_multiplier_combo,
                "x1\0""x2\0""x4\0"
            )) {
                config.set_settling_time_cycles_multiplier(std::next(AD5933::Masks::Or::SettlingTimeCyclesHB::multiplier_map.begin(), fields.settling_multiplier_combo)->first);
            }

            ImGui::Separator(); 

            if(ImGui::Combo("System Clock Source", &fields.sysclk_src_combo, "Internal\0""External\0")) {
                config.set_sysclk_src(std::next(AD5933::Masks::Or::Ctrl::LB::sysclk_src_map.begin(), fields.sysclk_src_combo)->first);
                fields.sysclk_freq = static_cast<uint32_t>(config.get_active_sysclk_freq());
            }

            ImGui::BeginDisabled();
            ImGui::Input_uint32_t("System Clock Frequency [Hz]", &fields.sysclk_freq, 0, 0, ImGuiInputTextFlags_ReadOnly);
            ImGui::EndDisabled();
        }

        const std::optional<Lock> Calibrate::draw_inner() {
            if(status == Status::Calibrating)  {
                ImGui::BeginDisabled();
                draw_input_fields();
                ImGui::EndDisabled();
            } else {
                draw_input_fields();
            }

            ImGui::Separator();

            if(status == Status::Calibrating) {
                ImGui::BeginDisabled();
                ImGui::Button("Calibrate", ImVec2(64 * GUI::Boilerplate::get_scale(), 0.0f));
                ImGui::EndDisabled();
            } else if(ImGui::Button("Calibrate", ImVec2(64 * GUI::Boilerplate::get_scale(), 0.0f))) {
                calibrate();
            }

            if(status == Status::Calibrating) {
                ImGui::SameLine();
                ImGui::ProgressBar(progress_bar_fraction, ImVec2(-FLT_MIN, 0), nullptr);
            }

            if(status != Status::Calibrated) {
                ImGui::BeginDisabled();
                ImGui::Button("Save", ImVec2(64 * GUI::Boilerplate::get_scale(), 0.0f));
                ImGui::EndDisabled();
            } else if(ImGui::Button("Save", ImVec2(64 * GUI::Boilerplate::get_scale(), 0.0f))) {
                save();
            }

            if(status == Status::Calibrating) {
                return Lock::Calibrate;
            } else {
                return std::nullopt;
            }
        }

        void Calibrate::draw(bool& enable, const ImGuiID side_id, Lock& lock) {
            if(first) {
                ImGui::DockBuilderDockWindow((const char*) name.c_str(), side_id);
            }

            if(ImGui::Begin((const char*) name.c_str(), &enable, ImGuiWindowFlags_NoMove) == false) {
                ImGui::End();
                return;
            }

            if(first) {
                ImGui::End();
                first = false;
                return;
            }

            if(lock != Lock::Released && lock != Lock::Calibrate) {
                ImGui::BeginDisabled();
                draw_inner();
                ImGui::EndDisabled();
            } else {
                const auto ret_lock { draw_inner() };
                if(lock == Lock::Calibrate && ret_lock.has_value() == false) {
                    lock = Lock::Released;
                } else if(ret_lock.has_value()) {
                    lock = ret_lock.value();
                }
            }

            ImGui::End();
        }

        void Calibrate::calibrate_cb(std::stop_token st, Calibrate& self) {
            self.shm->active_devices[self.index].information->clear();
            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature {
                    self.index,
                    Magic::Commands::Sweep::Configure{
                        self.config.to_raw_array()
                    }
                }
            );

            const uint16_t wished_size = self.config.get_num_of_inc().unwrap() + 1;
            const float progress_bar_step = 1.0f / static_cast<float>(wished_size);
            const float timeout_ms {
                (1.0f / static_cast<float>(self.config.get_start_freq().unwrap())) 
                * [&]() {
                    const float ret { static_cast<float>(self.config.get_settling_time_cycles_number().unwrap()) };
                    if(ret == 0) {
                        return 1.0f;
                    } else {
                        return ret;
                    }
                }()
                * AD5933::Masks::Or::SettlingTimeCyclesHB::get_multiplier_float(self.config.get_settling_time_cycles_multiplier())
                * 1'000'000.0f
                * 100.0f
            };
            const auto boost_timeout_ms { std::chrono::milliseconds(static_cast<size_t>(timeout_ms)) };
            const float impedance = self.fields.impedance;

            std::vector<AD5933::Data> tmp_raw_calibration;
            tmp_raw_calibration.reserve(wished_size);
            std::vector<AD5933::Calibration<float>> tmp_calibration;
            tmp_calibration.reserve(wished_size);

            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Commands::Sweep::Run{}
                }
            );

            do {
                const auto rx_payload {
                    self.shm->active_devices[self.index].information->read_for(
                        boost_timeout_ms
                    )
                };
                if(rx_payload.has_value() == false) {
                    self.status = Status::Failed;
                    std::cout << "ERROR: ESP32_AD5933: sweep: calibrate: timeout\n";
                    return;
                }

                if(st.stop_requested()) {
                    self.status = Status::NotCalibrated;
                    std::cout << "INFO: ESP32_AD5933: stopping calibration\n";
                    return;
                }

                bool is_valid_data = false;
                std::visit([&is_valid_data, &tmp_raw_calibration, &tmp_calibration, impedance](auto&& event) {
                    using T_Decay = std::decay_t<decltype(event)>;
                    if constexpr(std::is_same_v<T_Decay, Magic::Results::Sweep::ValidData>) {
                        std::array<uint8_t, 4> raw_array;
                        std::copy(event.real_imag_registers_data.begin(), event.real_imag_registers_data.end(), raw_array.begin());

                        const AD5933::Data tmp_data { raw_array };
                        tmp_raw_calibration.push_back(tmp_data);

                        const AD5933::Calibration<float> tmp_cal { tmp_data, impedance };
                        tmp_calibration.push_back(tmp_cal);

                        is_valid_data = true;
                    }
                }, rx_payload.value());

                if(is_valid_data == false) {
                    self.status = Status::Failed;
                    std::cout << "ERROR: ESP32_AD5933: calibration received wrong event result packet\n";
                    return;
                }
                
                self.progress_bar_fraction += progress_bar_step;
            } while(tmp_calibration.size() != wished_size);

            self.raw_calibration = tmp_raw_calibration;
            self.calibration = tmp_calibration;

            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Commands::Sweep::End{}
                }
            );

            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Commands::Debug::Start{}
                }
            );

            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Commands::Debug::CtrlHB{ static_cast<uint8_t>(AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode) }
                }
            );

            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Commands::Debug::End{}
                }
            );

            self.calibration_file = ns::CalibrationFile { impedance, self.config, tmp_calibration };
            self.calibration_queue_to_load_into_measurement->push(self.calibration_file);

            self.status = Status::Calibrated;
            self.plotted = false;
        }
        
        void Calibrate::calibrate() {
            status = Status::Calibrating;
            progress_bar_fraction = 0.0f;
            std::jthread t1(calibrate_cb, std::ref(*this));
            stop_source = t1.get_stop_source();
            t1.detach();
        }

        void Calibrate::save() const {
            nfdchar_t* outPath = nullptr;
            const std::array<nfdfilteritem_t, 1> filterItem { { "Calibration", "json" } };
            nfdresult_t result = NFD::SaveDialog(outPath, filterItem.data(), static_cast<nfdfiltersize_t>(filterItem.size()), nullptr, "calibration.json");
            if(result == NFD_OKAY) {
                const json j = calibration_file;
                std::ofstream(outPath) << std::setw(4) << j;
                if(outPath != nullptr) {
                    NFD::FreePath(outPath);
                    outPath = nullptr;
                }
            } else if(result == NFD_CANCEL) {
                std::printf("User pressed cancel!\n");
                if(outPath != nullptr) {
                    NFD::FreePath(outPath);
                    outPath = nullptr;
                }
                return;
            } else {
                std::printf("Error: %s\n", NFD::GetError());
                if(outPath != nullptr) {
                    NFD::FreePath(outPath);
                    outPath = nullptr;
                }
                return;
            }
        }

        Calibrate::Status Calibrate::get_status() const {
            return status;
        }
    }
}