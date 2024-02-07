#include "imgui_internal.h"

#include "imgui_custom/spinner.hpp"

#include "gui/windows/ble_client.hpp"

namespace GUI {
    namespace Windows {
        void ble_client(bool &enable, ImGuiID left_id, int &selected, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm, std::vector<Windows::Client>& client_windows) {
            if(enable == false) {
                return;
            }

            static bool first = true;
            if(first) {
                ImGui::DockBuilderDockWindow("BLE Client", left_id);
                first = false;
            }

            if(ImGui::Begin("BLE Client", &enable, ImGuiWindowFlags_NoMove) == false) {
                ImGui::End();
                return;
            }

            const auto show_table = [&selected, &shm]() {
                if(ImGui::BeginTable("Scan", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Identifier");
                    ImGui::TableNextColumn();
                    ImGui::Text("Address");
                    ImGui::TableNextColumn();
                    ImGui::Text("Status");
                    std::for_each(shm->discovery_devices->begin(), shm->discovery_devices->end(), [index = 0, &selected](const BLE_Client::Discovery::Device& e) mutable {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::PushID(index);
                        if(ImGui::Selectable(e.get_identifier().data(), index == selected, ImGuiSelectableFlags_SpanAllColumns)) {
                            if(selected == index) {
                                selected = -1;
                            } else {
                                selected = index;
                            }
                        }
                        ImGui::PopID();
                        ImGui::TableNextColumn();
                        ImGui::Text(e.get_address().data());
                        ImGui::TableNextColumn();
                        if(e.get_connected()) {
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
                        } else {
                            ImGui::Text("Disconnected");
                        }
                        index++;  
                    });
                    ImGui::EndTable();
                }
            };

            std::visit([&](auto&& active_state) {
                using T_Decay = std::decay_t<decltype(active_state)>;
                if constexpr (std::is_same_v<T_Decay, BLE_Client::StateMachines::Adapter::States::off>) {
                    if(ImGui::Button("Find adapter")) {
                        shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::turn_on{});
                    }
                } else if constexpr (std::is_same_v<T_Decay, BLE_Client::StateMachines::Adapter::States::on>) {
                    if(ImGui::Button("Scan")) {
                        shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::start_discovery{});
                    }
                } else if constexpr (std::is_same_v<T_Decay, BLE_Client::StateMachines::Adapter::States::discovering>) {
                    if(ImGui::Button("Stop")) {
                        shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::stop_discovery{});
                    }
                    ImGui::SameLine();
                    Spinner::Spinner("Scanning", 5.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                }
                if constexpr (!std::is_same_v<T_Decay, BLE_Client::StateMachines::Adapter::States::off>) {
                    show_table();
                    static bool connecting = false;
                    if(shm->discovery_devices->empty() == false && selected > -1 && connecting == false) {
                        if(ImGui::Button("Connect")) {
                            connecting = true;
                            const BLE_Client::StateMachines::Connector::Events::connect connect_event { shm->discovery_devices->at(selected).get_address() };
                            shm->cmd.send(connect_event);
                            std::thread([](auto shm, const BLE_Client::StateMachines::Connector::Events::connect connect_event, bool& connecting, std::vector<Windows::Client>& client_windows) {
                                for(int i = 0; i < 100; i++) {
                                    try{
                                        shm->attach_device(connect_event);
<<<<<<< HEAD
                                        client_windows.push_back(Windows::Client{std::string(connect_event.get_address()), client_windows.size() });
=======
                                        client_windows.push_back(Windows::Client{std::string(connect_event.get_address()), client_windows.size(), shm });
>>>>>>> origin/gui
                                        break;
                                    } catch(...) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                    }
                                }
                                connecting = false;
                            }, shm, connect_event, std::ref(connecting), std::ref(client_windows)).detach();
                        }
                    } else if(connecting == true) {
                        Spinner::Spinner("ClientSpinner", 5.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    }
                }
            }, *shm->active_state);

            /* filter names removed might add later down the line
            const auto to_lower = [](const std::string &string) { 
                auto ret = string;
                std::transform(ret.begin(), ret.end(), ret.begin(), [](unsigned char c) { return std::tolower(c); });
                return ret;
            };

            static char filter_content[32] = { '\0' };
            ImGui::InputText("##Filter", filter_content, sizeof(filter_content));
            if(std::strlen(filter_content) > 0) {
                std::vector<SimpleBLE::Peripheral> filtered_peripherals;
                filtered_peripherals.reserve(peripherals.size());
                std::string filter_string { to_lower(filter_content) };
                std::copy_if(peripherals.begin(), peripherals.end(), std::back_inserter(filtered_peripherals), [&to_lower, &filter_string](SimpleBLE::Peripheral e) {
                    if(to_lower(e.address().substr(0, filter_string.size())) == filter_string) {
                        return true;
                    } else if(to_lower(e.identifier().substr(0, filter_string.size())) == filter_string) {
                        return true;
                    } else {
                        return false;
                    }
                });
                show_connect_button(filtered_peripherals);
                show_table(filtered_peripherals);
            } else {
                show_connect_button(peripherals);
                show_table(peripherals);
            }
            */

            ImGui::End();
        }
    }
}
