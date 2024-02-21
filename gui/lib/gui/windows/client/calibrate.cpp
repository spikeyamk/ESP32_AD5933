#include <stdexcept>
#include <thread>
#include <atomic>

#include <nfd.hpp>
#include <utf/utf.hpp>
#include "imgui_internal.h"

#include "gui/boilerplate.hpp"
#include "ad5933/masks/maps.hpp"
#include "imgui_custom/spinner.hpp"
#include "imgui_custom/input_items.hpp"
#include "imgui_custom/char_filters.hpp"

#include "gui/windows/client/calibrate.hpp"

namespace GUI {
    namespace Windows {
        Calibrate::Calibrate(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) :
            index { index },
            shm{ shm }
        {
            name.append(utf::as_u8(std::to_string(index)));
        }

        void Calibrate::update_freq_start_valid() {
            try {
                const uint32_t tmp_freq_start = std::stoul(std::string(inputs.gui.freq_start));
                if(
                    tmp_freq_start < min_freq 
                    || tmp_freq_start >= max_freq
                ) {
                    valid_fields.freq_start = false;
                    return;
                }
                valid_fields.freq_start = true;
                inputs.numeric_values.freq_start = tmp_freq_start;
                config.set_start_freq(AD5933::uint_startfreq_t{ inputs.numeric_values.freq_start });
            } catch(const std::exception& e) {
                std::cout << "GUI::Windows::Calibrate::update_freq_start_valid: exception: " << e.what() << std::endl;
                valid_fields.freq_start = false;
            }
        }

        void Calibrate::update_freq_inc_valid() {
            try {
                const uint32_t tmp_freq_inc = std::stoul(std::string(inputs.gui.freq_inc));
                const uint32_t tmp_num_of_inc = std::stoul(std::string(inputs.gui.num_of_inc));
                if(
                    tmp_freq_inc >= max_freq
                    || tmp_freq_inc < res_freq
                    || ((inputs.numeric_values.freq_start + tmp_freq_inc) > max_freq)
                    || ((inputs.numeric_values.freq_start + (tmp_freq_inc * tmp_num_of_inc)) > max_freq)
                ) {
                    valid_fields.freq_inc = false;
                    return;
                }
                valid_fields.freq_inc = true;
                inputs.numeric_values.freq_inc = tmp_freq_inc;
                config.set_inc_freq(AD5933::uint_incfreq_t{ tmp_freq_inc });
            } catch(const std::exception& e) {
                std::cout << "GUI::Windows::Calibrate::update_freq_inc_valid: exception: " << e.what() << std::endl;
                valid_fields.freq_inc = false;
            }
        }

        void Calibrate::update_num_of_inc_valid() {
            try {
                const uint32_t tmp_freq_inc = static_cast<uint32_t>(std::stoul(std::string(inputs.gui.freq_inc)));
                const uint16_t tmp_num_of_inc = static_cast<uint16_t>(std::stoul(std::string(inputs.gui.num_of_inc)));
                if(
                    tmp_num_of_inc > max_9bit
                    || tmp_num_of_inc < 1
                    || ((inputs.numeric_values.freq_start + (tmp_freq_inc * tmp_num_of_inc)) > max_freq)
                ) {
                    valid_fields.num_of_inc = false;
                    return;
                }
                valid_fields.num_of_inc = true;
                inputs.numeric_values.num_of_inc = tmp_num_of_inc;
                config.set_num_of_inc(AD5933::uint9_t{ tmp_num_of_inc });
            } catch(const std::exception& e) {
                std::cout << "GUI::Windows::Calibrate::update_num_of_inc_valid: exception: " << e.what() << std::endl;
                valid_fields.num_of_inc = false;
            }
        }

        void Calibrate::update_impedance_valid() {
            if(inputs.numeric_values.impedance <= 0.0f) {
                valid_fields.impedance = false;
                return;
            }
            valid_fields.impedance = true;
        }

        void Calibrate::update_settling_number_valid() {
            try {
                const uint16_t tmp_settling_number = static_cast<uint16_t>(std::stoul(std::string(inputs.gui.settling_number)));
                if(tmp_settling_number > max_9bit) {
                    valid_fields.settling_number = false;
                    return;
                }
                inputs.numeric_values.settling_number = tmp_settling_number;
                valid_fields.settling_number = true;
                config.set_settling_time_cycles_number(AD5933::uint9_t{ tmp_settling_number });
            } catch(const std::exception& e) {
                std::cout << "GUI::Windows::Calibrate::update_settling_number_valid: exception: " << e.what() << std::endl;
                valid_fields.settling_number = false;
            }
        }
        
        std::atomic<float> progress_bar_fraction { 0.0f };

        void Calibrate::draw(bool& enable, const ImGuiID side_id) {
            if(first) {
                ImGui::DockBuilderDockWindow((const char*) name.c_str(), side_id);
            }

            if(ImGui::Begin((const char*) name.c_str(), &enable) == false) {
                ImGui::End();
                return;
            }

            if(first) {
                ImGui::End();
                first = false;
                return;
            }

            ImGui::Text("Sweep Parameters");
            ImGui::Separator();

            if(ImGui::InputTextValid(
                "Start Frequency [Hz]",
                inputs.gui.freq_start,
                sizeof(inputs.gui.freq_start),
                valid_fields.freq_start,
                ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter,
                ImGui::plus_minus_dot_char_filter_cb
            )) {
                update_freq_start_valid();
            }

            if(ImGui::InputTextValid(
                "Increment Frequency [Hz]",
                inputs.gui.freq_inc,
                sizeof(inputs.gui.freq_inc),
                valid_fields.freq_inc,
                ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter,
                ImGui::plus_minus_dot_char_filter_cb
            )) {
                update_freq_inc_valid();
            }

            if(ImGui::InputTextValid(
                "Number of Increments",
                inputs.gui.num_of_inc,
                sizeof(inputs.gui.num_of_inc),
                valid_fields.num_of_inc,
                ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter,
                ImGui::plus_minus_dot_char_filter_cb
            )) {
                update_num_of_inc_valid();
            }
            ImGui::Separator(); 

            if(ImGui::InputFloatValid(
                "Calibration Impedance [Ohm]",
                &inputs.numeric_values.impedance,
                valid_fields.impedance,
                0,
                0,
                "%.3f",
                ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter,
                ImGui::minus_char_filter_cb
            )) {
                update_impedance_valid();
            }
            ImGui::Separator(); 

            if(ImGui::Combo(
                "Output Excitation Voltage Range",
                &inputs.gui.voltage_range_combo,
                "2 Vppk\0""1 Vppk\0""400 mVppk\0""200 mVppk\0"
            )) {
                config.set_voltage_range(std::next(AD5933::Masks::Or::Ctrl::HB::voltage_map.begin(), inputs.gui.voltage_range_combo)->first);
            }

            if(ImGui::Combo("PGA Gain", &inputs.gui.pga_gain_combo, "x1\0""x5\0")) {
                config.set_PGA_gain(std::next(AD5933::Masks::Or::Ctrl::HB::pga_gain_map.begin(), inputs.gui.pga_gain_combo)->first);
            }
            ImGui::Separator(); 

            if(ImGui::Combo(
                "Settling Time Cycles Multiplier",
                &inputs.gui.settling_multiplier_combo,
                "x1\0""x2\0""x4\0"
            )) {
                config.set_settling_time_cycles_multiplier(std::next(AD5933::Masks::Or::SettlingTimeCyclesHB::multiplier_map.begin(), inputs.gui.settling_multiplier_combo)->first);
            }
            ImGui::Separator(); 

            if(ImGui::InputTextValid(
                "Settling Time Cycles Number",
                inputs.gui.settling_number,
                sizeof(inputs.gui.settling_number),
                valid_fields.settling_number,
                ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter,
                ImGui::plus_minus_dot_char_filter_cb
            )) {
                update_settling_number_valid();
            }

            if(ImGui::Combo("System Clock Source", &inputs.gui.sysclk_src_combo, "Internal\0""External\0")) {
                config.set_sysclk_src(std::next(AD5933::Masks::Or::Ctrl::LB::sysclk_src_map.begin(), inputs.gui.sysclk_src_combo)->first);
                inputs.numeric_values.sysclk_freq = static_cast<uint32_t>(config.get_active_sysclk_freq());
            }
            ImGui::Input_uint32_t("System Clock Frequency [Hz]", &inputs.numeric_values.sysclk_freq, 0, 0, ImGuiInputTextFlags_ReadOnly);
            ImGui::Separator();

            if(valid_fields.all_true() == false || status == Status::Calibrating) {
                ImGui::BeginDisabled();
                ImGui::Button("Calibrate");
                ImGui::EndDisabled();
            } else if(ImGui::Button("Calibrate")) {
                calibrate();
            }

            if(status == Status::Calibrating) {
                ImGui::SameLine();
                const float scale = GUI::Boilerplate::get_scale();
                Spinner::Spinner("Scanning", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
            }

            if(status != Status::Calibrated) {
                ImGui::BeginDisabled();
                ImGui::Button("Save");
                ImGui::EndDisabled();
            } else if(ImGui::Button("Save")) {
                save();
            }

            ImGui::End();
        }

        void Calibrate::calibrate_cb(
            std::stop_token st,
            Calibrate& calibrate_window
        ) {
            calibrate_window.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    calibrate_window.index,
                    Magic::Events::Commands::Sweep::Configure{
                        calibrate_window.config.to_raw_array()
                    }
                }
            );

            const uint16_t wished_size = calibrate_window.config.get_num_of_inc().unwrap() + 1;
            const float progress_bar_step = 1.0f / static_cast<float>(wished_size);
            const float timeout_ms =
                (1.0f / static_cast<float>(calibrate_window.config.get_start_freq().unwrap())) 
                * static_cast<float>(calibrate_window.config.get_settling_time_cycles_number().unwrap())
                * AD5933::Masks::Or::SettlingTimeCyclesHB::get_multiplier_float(calibrate_window.config.get_settling_time_cycles_multiplier())
                * 1'000'000.0f
                * 10.0f;
            const auto boost_timeout_ms { boost::posix_time::milliseconds(static_cast<size_t>(timeout_ms)) };
            const float impedance = calibrate_window.inputs.numeric_values.impedance;

            std::vector<AD5933::Data> tmp_raw_calibration;
            tmp_raw_calibration.reserve(wished_size);
            std::vector<AD5933::Calibration<float>> tmp_calibration;
            tmp_calibration.reserve(wished_size);

            calibrate_window.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    calibrate_window.index,
                    Magic::Events::Commands::Sweep::Run{}
                }
            );
            do {
                const auto rx_payload { calibrate_window.shm->active_devices[calibrate_window.index].information->read_for(boost_timeout_ms) };
                if(rx_payload.has_value() == false) {
                    calibrate_window.status = Status::Failed;
                    std::cout << "ERROR: ESP32_AD5933: sweep: calibrate: timeout\n";
                    return;
                }

                if(st.stop_requested()) {
                    calibrate_window.status = Status::NotCalibrated;
                    std::cout << "INFO: ESP32_AD5933: stopping calibration\n";
                    return;
                }

                bool is_valid_data = false;
                std::visit([&is_valid_data, &tmp_raw_calibration, &tmp_calibration, impedance](auto&& event) {
                    using T_Decay = std::decay_t<decltype(event)>;
                    if constexpr(std::is_same_v<T_Decay, Magic::Events::Results::Sweep::ValidData>) {
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
                    calibrate_window.status = Status::Failed;
                    std::cout << "ERROR: ESP32_AD5933: calibration received wrong event result packet\n";
                    return;
                }
                
               progress_bar_fraction.fetch_add(progress_bar_step);
            } while(tmp_calibration.size() != wished_size);

            calibrate_window.raw_calibration = tmp_raw_calibration;
            calibrate_window.calibration = tmp_calibration;

            calibrate_window.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    calibrate_window.index,
                    Magic::Events::Commands::Sweep::End{}
                }
            );

            calibrate_window.status = Status::Calibrated;
            calibrate_window.plotted = false;
        }
        
        void Calibrate::calibrate() {
            status = Status::Calibrating;
            std::jthread t1(calibrate_cb, std::ref(*this));
            stop_source = t1.get_stop_source();
            t1.detach();
        }

        void Calibrate::save() const {
            nfdchar_t* outPath = nullptr;
            const std::array<nfdfilteritem_t, 1> filterItem { { "Calibration", "json" } };
            nfdresult_t result = NFD::SaveDialog(outPath, filterItem.data(), static_cast<nfdfiltersize_t>(filterItem.size()), nullptr, "calibration.json");
            if(result == NFD_OKAY) {
                const ns::CalibrationFile calibration_file { inputs.numeric_values.impedance, config, calibration };
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