#include <thread>
#include <fmt/core.h>
#include <fmt/color.h>

#include "gui/spinner.hpp"

#include "gui/windows/sweep.hpp"

namespace GUI {
    namespace Windows {
        void send_configure(Client &client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
            shm->send_packet_and_footer(client.index, client.configure_captures.config.to_raw_array(), Magic::Packets::FrequencySweep::configure);
            client.configured = true;
        }

        void calibrate(std::stop_token st, Client &client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
            const uint16_t wished_size = client.configure_captures.config.get_num_of_inc().unwrap() + 1;
            const float progress_bar_step = 1.0f / static_cast<float>(wished_size);
            const float timeout_ms = (1.0f / static_cast<float>(client.configure_captures.config.get_start_freq().unwrap())) 
                * static_cast<float>(client.configure_captures.config.get_settling_time_cycles_number().unwrap())
                * AD5933::Masks::Or::SettlingTimeCyclesHB::get_multiplier_float(client.configure_captures.config.get_settling_time_cycles_multiplier())
                * 1'000'000.0f
                * 10.0f;
            const auto boost_timeout_ms { boost::posix_time::milliseconds(static_cast<size_t>(timeout_ms)) };

            std::vector<AD5933::Data> tmp_raw_calibration;
            tmp_raw_calibration.reserve(wished_size);
            std::vector<AD5933::Calibration<float>> tmp_calibration;
            tmp_calibration.reserve(wished_size);

            client.progress_bar_fraction = 0.0f;

            shm->send_packet(client.index, Magic::Packets::FrequencySweep::run);

            do {
                const auto rx_payload { shm->notify_channels[client.index]->read_for(boost_timeout_ms) };
                if(rx_payload.has_value() == false) {
                    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: sweep: calibrate: timeout\n");
                    return;
                }

                if(st.stop_requested()) {
                    client.calibrating = false;
    		        fmt::print(fmt::fg(fmt::color::yellow), "INFO: ESP32_AD5933: stopping calibration\n");
                    return;
                }

                std::array<uint8_t, 4> raw_array;
                std::copy(rx_payload.value().begin(), rx_payload.value().begin() + 4, raw_array.begin());
                AD5933::Data tmp_data { raw_array };
                tmp_raw_calibration.push_back(tmp_data);
                AD5933::Calibration<float> tmp_cal { tmp_data, client.configure_captures.calibration_impedance };
                tmp_calibration.push_back(tmp_cal);
                client.progress_bar_fraction += progress_bar_step;
            } while(tmp_calibration.size() != wished_size);
            client.raw_calibration = tmp_raw_calibration;
            client.calibration = tmp_calibration;
            client.calibrated = true;
            client.calibrating = false;
        }
    }

    namespace Windows {
        void run_sweep(std::stop_token st, Client &client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
            const uint16_t wished_size = client.configure_captures.config.get_num_of_inc().unwrap() + 1;
            assert(wished_size <= client.calibration.size());
            const float progress_bar_step = 1.0f / static_cast<float>(wished_size);
            const float timeout_ms = (1.0f / static_cast<float>(client.configure_captures.config.get_start_freq().unwrap())) 
                * static_cast<float>(client.configure_captures.config.get_settling_time_cycles_number().unwrap())
                * AD5933::Masks::Or::SettlingTimeCyclesHB::get_multiplier_float(client.configure_captures.config.get_settling_time_cycles_multiplier())
                * 1'000'000.0f
                * 10.0f;
            const auto boost_timeout_ms { boost::posix_time::milliseconds(static_cast<size_t>(timeout_ms)) };
            const std::vector<AD5933::Calibration<float>> tmp_calibration { client.calibration };
            do {
                std::vector<AD5933::Data> tmp_raw_measurement;
                tmp_raw_measurement.reserve(wished_size);
                std::vector<AD5933::Measurement<float>> tmp_measurement;
                tmp_measurement.reserve(wished_size);

                client.progress_bar_fraction = 0.0f;

                shm->send_packet(client.index, Magic::Packets::FrequencySweep::run);
                do {
                    const auto rx_payload { shm->notify_channels[client.index]->read_for(boost_timeout_ms) };
                    if(rx_payload.has_value() == false) {
                        fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: sweep: failed: timeout\n");
                        return;
                    }

                    if(st.stop_requested()) {
                        client.sweeping = false;
                        fmt::print(fmt::fg(fmt::color::yellow), "INFO: ESP32_AD5933: stopping sweep\n");
                        return;
                    }

                    std::array<uint8_t, 4> raw_array;
                    std::copy(rx_payload.value().begin(), rx_payload.value().begin() + 4, raw_array.begin());
                    AD5933::Data tmp_data { raw_array };
                    tmp_raw_measurement.push_back(tmp_data);
                    AD5933::Measurement<float> tmp_meas { tmp_data, tmp_calibration[tmp_measurement.size()] };
                    tmp_measurement.push_back(tmp_meas);
                    client.progress_bar_fraction += progress_bar_step;
                } while(tmp_measurement.size() != wished_size);
                client.raw_measurement = tmp_raw_measurement;
                client.measurement = tmp_measurement;
                client.sweeped = true;
            } while(client.periodically_sweeping);
            client.sweeping = false;
        }


        void sweep(int i, ImGuiID side_id, bool &enable, Client &client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
            static char base[] = "Configure";
            char name[20];
            std::sprintf(name, "%s##%d", base, i);

            static int first = 0;
            if(first == i) {
                ImGui::DockBuilderDockWindow(name, side_id);
                first++;
            }

            if(ImGui::Begin(name, &enable) == false) {
                ImGui::End();
                return;
            }

            const ImGuiInputTextFlags input_flags = (client.calibrating == true || client.sweeping == true) ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None;

            ImGui::Separator(); 
            ImGui::Text("Sweep Parameters");

            ImGui::Separator();

            ImGui::InputText(
                "Start Frequency",
                client.configure_captures.freq_start.data(),
                client.configure_captures.freq_start.size(),
                input_flags
            );
            ImGui::InputText(
                "Increment Frequency",
                client.configure_captures.freq_inc.data(),
                client.configure_captures.freq_inc.size(),
                input_flags
            );
            ImGui::InputText(
                "Number of Increments",
                client.configure_captures.num_of_inc.data(),
                client.configure_captures.num_of_inc.size(),
                input_flags
            );

            ImGui::Separator();

            ImGui::InputFloat(
                "Calibration impedance",
                &client.configure_captures.calibration_impedance,
                0.0f,
                0.0f,
                "%f",
                input_flags
            );

            ImGui::Separator(); 

            ImGui::Combo(
                "Output Excitation Voltage Range",
                &client.configure_captures.voltage_range_combo,
                "2 Vppk\0""1 Vppk\0""400 mVppk\0""200 mVppk\0"
            );

            ImGui::Separator();

            ImGui::Combo("PGA Gain", &client.configure_captures.pga_gain_combo, "x1\0""x5\0");

            ImGui::Separator();

            ImGui::InputText(
                "Number of Settling Time Cycles",
                client.configure_captures.settling_time_cycles_num.data(),
                client.configure_captures.settling_time_cycles_num.size(),
                input_flags
            );
            ImGui::Combo(
                "Settling Time Cycles Multiplier",
                &client.configure_captures.settling_time_cycles_multiplier_combo,
                "x1\0""x2\0""x4\0"
            );

            ImGui::Separator();

            ImGui::Combo("System Clock Source", &client.configure_captures.sysclk_src_combo, "Internal\0""External\0");
            ImGui::InputText(
                "System Clock Frequency",
                client.configure_captures.sysclk_freq.data(),
                client.configure_captures.sysclk_freq.size(),
                ImGuiInputTextFlags_ReadOnly
            );

            ImGui::Separator(); 

            if(client.debug_started == true) {
                client.configure_captures.update_config();
                ImGui::End();
                return;
            }

            if(ImGui::Button("Configure")) {
                std::jthread t1(send_configure, std::ref(client), shm);
                t1.detach();
            }

            if(client.configured == true) {
                ImGui::SameLine();
                if(ImGui::Button("Freq Sweep End")) {
                    std::jthread t1([](Client& client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
                        shm->send_packet(client.index, Magic::Packets::FrequencySweep::end);
                        client.configured = false;
                    }, std::ref(client), shm);
                    t1.detach();
                } 

                if(client.calibrating == false && client.sweeping == false) {
                    if(ImGui::Button("Calibrate")) {
                        client.calibrating = true;
                        client.calibrated = false;
                        std::jthread t1(calibrate, client.ss.get_token(), std::ref(client), shm);
                        t1.detach();
                    }
                } else if(client.sweeping == false) {
                    Spinner::Spinner("Calibrating", 10.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    ImGui::ProgressBar(client.progress_bar_fraction);
                }

                if(client.calibrated == true) {
                    ImGui::SameLine();
                    ImGui::Checkbox("Periodic Sweep", &client.periodically_sweeping);
                    if(client.sweeping == false) {
                        if(ImGui::Button("Start Sweep")) {
                            client.sweeping = true;
                            client.sweeped = false;
                            std::jthread t1(run_sweep, client.ss.get_token(), std::ref(client), shm);
                            t1.detach();
                        }
                    } else if(client.calibrating == false) {
                        Spinner::Spinner("Measuring", 10.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                        ImGui::ProgressBar(client.progress_bar_fraction);
                    }
                }
            }

            client.configure_captures.update_config();
            ImGui::End();
        }

   
    }
}