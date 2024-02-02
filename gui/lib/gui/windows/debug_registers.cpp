#include <thread>

#include <fmt/core.h>
#include <fmt/color.h>
#include "imgui_internal.h"

#include "gui/windows/client.hpp"
#include "magic/events/commands.hpp"
#include "magic/events/results.hpp"

namespace GUI {
    namespace Windows {
        bool dump(Client &client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_event{ client.index, Magic::Events::Commands::Debug::Dump{} });
            const auto rx_payload { shm->notify_channels[client.index]->read_for(boost::posix_time::milliseconds(1'000)) };

            if(rx_payload.has_value() == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: Debug: Dump failed: timeout\n");
                return false;
            }

            bool is_dump_all_registers = false;
            std::visit([&is_dump_all_registers, &client](auto&& event_result) {
                using T_Decay = std::decay_t<decltype(event_result)>;
                if constexpr(std::is_same_v<T_Decay, Magic::Events::Results::Debug::Dump>) {
                    is_dump_all_registers = true;
                    client.debug_captures.update_captures(std::string(event_result.registers_data.begin(), event_result.registers_data.end()));
                }
            }, rx_payload.value());

            if(is_dump_all_registers == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: Debug: Dump failed: Didn't get the right packet\n");
                return false;
            }

            return true;
        }

        void debug_program(Client &client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
            shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_event{
                    client.index,
                    Magic::Events::Commands::Debug::Program{
                        client.debug_captures.config.to_raw_array()
                    }
                }
            );
        }
        
        void program_and_dump(Client &client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
            debug_program(client, shm);
            if(dump(client, shm) == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: program_and_dump: dump: failed\n");
            }
        }

        void command_and_dump(Client &client, AD5933::Masks::Or::Ctrl::HB::Command command, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
            shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_event{
                    client.index,
                    Magic::Events::Commands::Debug::CtrlHB{
                        static_cast<uint8_t>(command)
                    }
                }
            );
            if(dump(client, shm) == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: command_and_dump: dump: {} failed\n", static_cast<uint8_t>(command));
            }
        }
    }

    namespace Windows {
        void debug_registers(int i, ImGuiID side_id, bool &enable, Client &client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
            std::string name = "Debug Registers##" + std::to_string(i);
            static int first = 0;
            if(first == i) {
                ImGui::DockBuilderDockWindow(name.c_str(), side_id);
                first++;
            }

            if(ImGui::Begin(name.c_str(), &enable) == false) {
                ImGui::End();
                return;
            }

            if(client.configured == false) {
                if(client.debug_started == false) {
                    if(ImGui::Button("Start")) {
                        shm->cmd.send(
                            BLE_Client::StateMachines::Connection::Events::write_event{
                                client.index,
                                Magic::Events::Commands::Debug::Start{}
                            }
                        );
                        client.debug_started = true;
                    }
                } else {
                    if(ImGui::Button("Dump")) {
                        dump(client, shm);
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Program")) {
                        program_and_dump(client, shm);
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("End")) {
                        shm->cmd.send(
                            BLE_Client::StateMachines::Connection::Events::write_event{
                                client.index,
                                Magic::Events::Commands::Debug::End{}
                            }
                        );
                        client.debug_started = false;
                    }
                }
            }

            ImGui::Separator();

            ImGui::Text("ControlHB:");
            ImGui::Text("\tControlHB Command: %s", AD5933::Masks::get_map_str(client.debug_captures.config.get_command(), AD5933::Masks::Or::Ctrl::HB::command_map));
            ImGui::Text("\tExcitation Output Voltage Range: %s", AD5933::Masks::get_map_str(client.debug_captures.config.get_voltage_range(), AD5933::Masks::Or::Ctrl::HB::voltage_map));
            ImGui::Text("\tPGA Gain: %s", AD5933::Masks::get_map_str(client.debug_captures.config.get_PGA_gain(), AD5933::Masks::Or::Ctrl::HB::pga_gain_map));
            ImGui::Separator();

            ImGui::Text("ControlLB:");
            ImGui::Text("\tSystem Clock Source: %s", AD5933::Masks::get_map_str(client.debug_captures.config.get_sysclk_src(), AD5933::Masks::Or::Ctrl::LB::sysclk_src_map)); 
            ImGui::Separator();

            ImGui::Text("Start Frequency: %f", client.debug_captures.config.get_start_freq().to_float());
            ImGui::Text("Frequency Increment: %f", client.debug_captures.config.get_inc_freq().to_float());
            ImGui::Text("Number of Increments: %u", client.debug_captures.config.get_num_of_inc().unwrap());
            ImGui::Text("Number of Settling Time Cycles: %u", client.debug_captures.config.get_settling_time_cycles_number().unwrap());
            ImGui::Text("Settling Time Cycles Multiplier: %s", AD5933::Masks::get_map_str(client.debug_captures.config.get_settling_time_cycles_multiplier(), AD5933::Masks::Or::SettlingTimeCyclesHB::multiplier_map));
            ImGui::Separator();
            ImGui::Text("Status: %s", AD5933::Masks::get_map_str(client.debug_captures.data.get_status(), AD5933::Masks::Or::status_map));
            ImGui::Separator();

            ImGui::InputText("Temperature [°C]",
                std::to_string(client.debug_captures.data.get_temperature()).data(),
                std::to_string(client.debug_captures.data.get_temperature()).size(),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::InputText("Real Part [1/Ohm]",
                std::to_string(client.debug_captures.data.get_real_part()).data(),
                std::to_string(client.debug_captures.data.get_real_part()).size(),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::InputText("Imaginary Part [1/Ohm]",
                std::to_string(client.debug_captures.data.get_imag_part()).data(),
                std::to_string(client.debug_captures.data.get_imag_part()).size(),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::InputText("Raw Magnitude [1/Ohm]",
                std::to_string(client.debug_captures.data.get_raw_magnitude()).data(),
                std::to_string(client.debug_captures.data.get_raw_magnitude()).size(),
                ImGuiInputTextFlags_ReadOnly
            );
            static const auto float_to_str_lambda = [](double number) -> std::string {
                std::ostringstream stream;
                // Calculate the magnitude of the number
                double magnitude = std::abs(number);
                int precision = (magnitude == 0.0f) ? 0 : -static_cast<int>(std::floor(std::log10(magnitude))) + 7;
                stream << std::setprecision(precision) << number;
                return stream.str();
            };
            ImGui::InputText("Raw Phase [rad]",
                float_to_str_lambda(client.debug_captures.data.get_raw_phase()).data(),
                float_to_str_lambda(client.debug_captures.data.get_raw_phase()).size(),
                ImGuiInputTextFlags_ReadOnly
            );

            ImGui::Separator();

            ImGui::Text("Read/Write Registers");
            {
                ImGui::Separator();
                ImGui::InputText("CTRL_HB (0x80)", client.debug_captures.ctrl_HB.data(), client.debug_captures.ctrl_HB.size());
                ImGui::InputText("CTRL_LB (0x81)", client.debug_captures.ctrl_LB.data(), client.debug_captures.ctrl_LB.size());
                ImGui::Separator();
                ImGui::InputText("FREQ_START_HB (0x82)", client.debug_captures.freq_start_HB.data(), client.debug_captures.freq_start_HB.size());
                ImGui::InputText("FREQ_START_MB (0x83)", client.debug_captures.freq_start_MB.data(), client.debug_captures.freq_start_MB.size());
                ImGui::InputText("FREQ_START_LB (0x84)", client.debug_captures.freq_start_LB.data(), client.debug_captures.freq_start_LB.size());
                ImGui::Separator();
                ImGui::InputText("FREQ_INC_HB (0x85)", client.debug_captures.freq_inc_HB.data(), client.debug_captures.freq_inc_HB.size());
                ImGui::InputText("FREQ_INC_MB (0x86)", client.debug_captures.freq_inc_MB.data(), client.debug_captures.freq_inc_MB.size());
                ImGui::InputText("FREQ_INC_LB (0x87)", client.debug_captures.freq_inc_LB.data(), client.debug_captures.freq_inc_LB.size());
                ImGui::Separator();
                ImGui::InputText("NUM_OF_INC_HB (0x88)", client.debug_captures.num_of_inc_HB.data(), client.debug_captures.num_of_inc_HB.size());
                ImGui::InputText("NUM_OF_INC_LB (0x89)", client.debug_captures.num_of_inc_LB.data(), client.debug_captures.num_of_inc_LB.size());
                ImGui::Separator();
                ImGui::InputText("NUM_OF_SETTLING_TIME_CYCLES_HB (0x8A)", client.debug_captures.settling_time_cycles_HB.data(), client.debug_captures.settling_time_cycles_HB.size());
                ImGui::InputText("NUM_OF_SETTLING_TIME_CYCLES_LB (0x8B)", client.debug_captures.settling_time_cycles_LB.data(), client.debug_captures.settling_time_cycles_LB.size());

                ImGui::Separator();

            {
                ImGui::Text("Send Control Register Command Controls");
                if(ImGui::Button("Power-down mode")) {
                    std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode, shm);
                    t1.detach();
                } ImGui::SameLine();
                    if(ImGui::Button("Standby mode")) {
                        std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::StandbyMode, shm);
                        t1.detach();
                    } ImGui::SameLine();
                    if(ImGui::Button("No operation (NOP_0)")) {
                        std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::Nop_0, shm);
                        t1.detach();
                    }

                if(ImGui::Button("Measure temperature")) {
                    std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::MeasureTemp, shm);
                    t1.detach();
                }

                if(ImGui::Button("Initialize with start frequency")) {
                    std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::InitStartFreq, shm);
                    t1.detach();
                } ImGui::SameLine();
                    if(ImGui::Button("Start frequency sweep")) {
                        std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::StartFreqSweep, shm);
                        t1.detach();
                    } ImGui::SameLine();
                    if(ImGui::Button("Increment frequency")) {
                        std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::IncFreq, shm);
                        t1.detach();
                    } ImGui::SameLine();
                    if(ImGui::Button("Repeat frequency")) {
                        std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::RepeatFreq, shm);
                        t1.detach();
                    }
                }

                ImGui::Separator();

                ImGui::Text("Special Status Register");
                ImGui::InputText("STATUS (0x8F)", client.debug_captures.status_capture.data(), client.debug_captures.status_capture.size(), ImGuiInputTextFlags_ReadOnly);

                ImGui::Separator();

                ImGui::Text("Read-only Data Registers");
                ImGui::InputText("TEMP_DATA_HB (0x92)", client.debug_captures.temp_data_HB.data(), client.debug_captures.temp_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::InputText("TEMP_DATA_LB (0x93)", client.debug_captures.temp_data_LB.data(), client.debug_captures.temp_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::Separator();
                ImGui::InputText("REAL_DATA_HB (0x94)", client.debug_captures.real_data_HB.data(), client.debug_captures.real_data_HB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::InputText("REAL_DATA_LB (0x95)", client.debug_captures.real_data_LB.data(), client.debug_captures.real_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::Separator();
                ImGui::InputText("IMAG_DATA_HB (0x96)", client.debug_captures.imag_data_HB.data(), client.debug_captures.imag_data_HB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::InputText("IMAG_DATA_LB (0x97)", client.debug_captures.imag_data_LB.data(), client.debug_captures.imag_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::Separator();
            }

            client.debug_captures.update_config();

            ImGui::End();
        }
    }
}
