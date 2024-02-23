#include <iostream>
#include <thread>
#include <stdexcept>

#include <fmt/core.h>
#include <fmt/color.h>
#include "imgui_internal.h"

#include "magic/events/commands.hpp"
#include "magic/events/results.hpp"
#include "misc/variant_tester.hpp"
#include "ad5933/masks/masks.hpp"

#include "gui/windows/client/debug.hpp"

namespace GUI {
    namespace Windows {
        Debug::Status Debug::get_status() const {
            return status;
        }

        Debug::Debug(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) :
            index { index },
            shm { shm }
        {
            name.append(std::to_string(index));
        }

        void Debug::draw_input_elements() {
            ImGui::SameLine();

            ImGui::Button("Program");

            ImGui::Separator();

            ImGui::Text("ControlHB:");
            ImGui::Text("\tControlHB Command: %s", AD5933::Masks::get_map_str(debug_captures.config.get_command(), AD5933::Masks::Or::Ctrl::HB::command_map));
            ImGui::Text("\tExcitation Output Voltage Range: %s", AD5933::Masks::get_map_str(debug_captures.config.get_voltage_range(), AD5933::Masks::Or::Ctrl::HB::voltage_map));
            ImGui::Text("\tPGA Gain: %s", AD5933::Masks::get_map_str(debug_captures.config.get_PGA_gain(), AD5933::Masks::Or::Ctrl::HB::pga_gain_map));
            ImGui::Separator();

            ImGui::Text("ControlLB:");
            ImGui::Text("\tSystem Clock Source: %s", AD5933::Masks::get_map_str(debug_captures.config.get_sysclk_src(), AD5933::Masks::Or::Ctrl::LB::sysclk_src_map)); 
            ImGui::Separator();

            ImGui::Text("Start Frequency: %f", debug_captures.config.get_start_freq().to_float());
            ImGui::Text("Frequency Increment: %f", debug_captures.config.get_inc_freq().to_float());
            ImGui::Text("Number of Increments: %u", debug_captures.config.get_num_of_inc().unwrap());
            ImGui::Text("Number of Settling Time Cycles: %u", debug_captures.config.get_settling_time_cycles_number().unwrap());
            ImGui::Text("Settling Time Cycles Multiplier: %s", AD5933::Masks::get_map_str(debug_captures.config.get_settling_time_cycles_multiplier(), AD5933::Masks::Or::SettlingTimeCyclesHB::multiplier_map));
            ImGui::Separator();
            ImGui::Text("Status: %s", AD5933::Masks::get_map_str(debug_captures.data.get_status(), AD5933::Masks::Or::status_map));
            ImGui::Separator();

            ImGui::InputText("Temperature [°C]",
                std::to_string(debug_captures.data.get_temperature()).data(),
                std::to_string(debug_captures.data.get_temperature()).size(),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::InputText("Real Part [1/Ohm]",
                std::to_string(debug_captures.data.get_real_part()).data(),
                std::to_string(debug_captures.data.get_real_part()).size(),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::InputText("Imaginary Part [1/Ohm]",
                std::to_string(debug_captures.data.get_imag_part()).data(),
                std::to_string(debug_captures.data.get_imag_part()).size(),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::InputText("Raw Magnitude [1/Ohm]",
                std::to_string(debug_captures.data.get_raw_magnitude()).data(),
                std::to_string(debug_captures.data.get_raw_magnitude()).size(),
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
                float_to_str_lambda(debug_captures.data.get_raw_phase()).data(),
                float_to_str_lambda(debug_captures.data.get_raw_phase()).size(),
                ImGuiInputTextFlags_ReadOnly
            );

            ImGui::Separator();

            ImGui::Text("Read/Write Registers");
            {
                ImGui::Separator();
                ImGui::InputText("CTRL_HB (0x80)", debug_captures.ctrl_HB.data(), debug_captures.ctrl_HB.size());
                ImGui::InputText("CTRL_LB (0x81)", debug_captures.ctrl_LB.data(), debug_captures.ctrl_LB.size());
                ImGui::Separator();
                ImGui::InputText("FREQ_START_HB (0x82)", debug_captures.freq_start_HB.data(), debug_captures.freq_start_HB.size());
                ImGui::InputText("FREQ_START_MB (0x83)", debug_captures.freq_start_MB.data(), debug_captures.freq_start_MB.size());
                ImGui::InputText("FREQ_START_LB (0x84)", debug_captures.freq_start_LB.data(), debug_captures.freq_start_LB.size());
                ImGui::Separator();
                ImGui::InputText("FREQ_INC_HB (0x85)", debug_captures.freq_inc_HB.data(), debug_captures.freq_inc_HB.size());
                ImGui::InputText("FREQ_INC_MB (0x86)", debug_captures.freq_inc_MB.data(), debug_captures.freq_inc_MB.size());
                ImGui::InputText("FREQ_INC_LB (0x87)", debug_captures.freq_inc_LB.data(), debug_captures.freq_inc_LB.size());
                ImGui::Separator();
                ImGui::InputText("NUM_OF_INC_HB (0x88)", debug_captures.num_of_inc_HB.data(), debug_captures.num_of_inc_HB.size());
                ImGui::InputText("NUM_OF_INC_LB (0x89)", debug_captures.num_of_inc_LB.data(), debug_captures.num_of_inc_LB.size());
                ImGui::Separator();
                ImGui::InputText("NUM_OF_SETTLING_TIME_CYCLES_HB (0x8A)", debug_captures.settling_time_cycles_HB.data(), debug_captures.settling_time_cycles_HB.size());
                ImGui::InputText("NUM_OF_SETTLING_TIME_CYCLES_LB (0x8B)", debug_captures.settling_time_cycles_LB.data(), debug_captures.settling_time_cycles_LB.size());

                ImGui::Separator();

            {
                ImGui::Text("Send Control Register Command Controls");
                if(ImGui::Button("Power-down mode")) {
                    command_and_dump(AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode);
                } ImGui::SameLine();
                    if(ImGui::Button("Standby mode")) {
                        command_and_dump(AD5933::Masks::Or::Ctrl::HB::Command::StandbyMode);
                    } ImGui::SameLine();
                    if(ImGui::Button("No operation (NOP_0)")) {
                       command_and_dump(AD5933::Masks::Or::Ctrl::HB::Command::Nop_0);
                    }

                if(ImGui::Button("Measure temperature")) {
                    command_and_dump(AD5933::Masks::Or::Ctrl::HB::Command::MeasureTemp);
                }

                if(ImGui::Button("Initialize with start frequency")) {
                    command_and_dump(AD5933::Masks::Or::Ctrl::HB::Command::InitStartFreq);
                } ImGui::SameLine();
                    if(ImGui::Button("Start frequency sweep")) {
                        command_and_dump(AD5933::Masks::Or::Ctrl::HB::Command::StartFreqSweep);
                    } ImGui::SameLine();
                    if(ImGui::Button("Increment frequency")) {
                        command_and_dump(AD5933::Masks::Or::Ctrl::HB::Command::IncFreq);
                    } ImGui::SameLine();
                    if(ImGui::Button("Repeat frequency")) {
                        command_and_dump(AD5933::Masks::Or::Ctrl::HB::Command::RepeatFreq);
                    }
                }

                ImGui::Separator();

                ImGui::Text("Special Status Register");
                ImGui::InputText("STATUS (0x8F)", debug_captures.status_capture.data(), debug_captures.status_capture.size(), ImGuiInputTextFlags_ReadOnly);

                ImGui::Separator();

                ImGui::Text("Read-only Data Registers");
                ImGui::InputText("TEMP_DATA_HB (0x92)", debug_captures.temp_data_HB.data(), debug_captures.temp_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::InputText("TEMP_DATA_LB (0x93)", debug_captures.temp_data_LB.data(), debug_captures.temp_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::Separator();
                ImGui::InputText("REAL_DATA_HB (0x94)", debug_captures.real_data_HB.data(), debug_captures.real_data_HB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::InputText("REAL_DATA_LB (0x95)", debug_captures.real_data_LB.data(), debug_captures.real_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::Separator();
                ImGui::InputText("IMAG_DATA_HB (0x96)", debug_captures.imag_data_HB.data(), debug_captures.imag_data_HB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::InputText("IMAG_DATA_LB (0x97)", debug_captures.imag_data_LB.data(), debug_captures.imag_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::Separator();
            }

            debug_captures.update_config();
        }

        void Debug::draw(bool &enable, const ImGuiID side_id) {
            if(first) {
                ImGui::DockBuilderDockWindow(name.c_str(), side_id);
                first = false;
            }

            if(ImGui::Begin(name.c_str(), &enable) == false) {
                ImGui::End();
                return;
            }

            if(ImGui::Button("Dump")) {
                if(dump()) {
                    status = Status::Dumped;
                }
            }

            if(status == Status::NotDumped) {
                ImGui::BeginDisabled();
                draw_input_elements();
                ImGui::EndDisabled();
            } else {
                draw_input_elements();
            }

            ImGui::End();
        }

        bool Debug::dump() {
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::Debug::Start{} });
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::Debug::Dump{} });
            const auto rx_payload { shm->active_devices[index].information->read_for(boost::posix_time::milliseconds(1'000)) };

            if(rx_payload.has_value() == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: GUI::Windows::Debug::dump: timeout\n");
                return false;
            }

            if(variant_tester<Magic::Events::Results::Debug::Dump>(rx_payload.value()) == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: GUI::Windows::Debug::dump: rx_payload: wrong variant type\n");
                return false;
            }

            try {
                std::visit([&](auto&& event) {
                    if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Events::Results::Debug::Dump>) {
                        debug_captures.update_captures(std::string(event.registers_data.begin(), event.registers_data.end()));
                    }
                }, rx_payload.value());

                shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::Debug::End{} });
                return true;
            } catch(const std::exception& e) {
    		    std::cout << "ERROR: GUI::Windows::Debug::dump: exception: " << e.what() << std::endl;
                return false;
            }
        }

        bool Debug::program_and_dump() {
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::Debug::Start{} });
            shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    index,
                    Magic::Events::Commands::Debug::Program{
                        debug_captures.config.to_raw_array()
                    }
                }
            );
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::Debug::End{} });
            return dump();
        }

        bool Debug::command_and_dump(const AD5933::Masks::Or::Ctrl::HB::Command command) {
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::Debug::Start{} });
            shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    index,
                    Magic::Events::Commands::Debug::CtrlHB{
                        static_cast<uint8_t>(command)
                    }
                }
            );
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::Debug::End{} });
            return dump();
        }
    }
}