#include <thread>

#include "imgui_internal.h"

#include "imgui_custom/spinner.hpp"
#include "gui/boilerplate.hpp"

#include "gui/windows/ble_adapter.hpp"

namespace GUI {
    namespace Windows {
        BLE_Adapter::BLE_Adapter(std::shared_ptr<BLE_Client::SHM::ParentSHM> shm, std::vector<Windows::Client>& client_windows) :
            shm { shm },
            client_windows { client_windows }
        {}

        void BLE_Adapter::draw(bool &enable, const ImGuiID left_id) {
            if(enable == false) {
                return;
            }

            if(first) {
                ImGui::DockBuilderDockWindow((const char*) name.data(), left_id);
                first = false;
            }

            if(ImGui::Begin((const char*) name.data(), &enable, ImGuiWindowFlags_NoMove) == false) {
                ImGui::End();
                return;
            }

            const auto show_disabled_connect_button = []() {
                ImGui::BeginDisabled();
                ImGui::Button("Connect");
                ImGui::EndDisabled();
            };

            std::visit([&](auto&& active_state) {
                using T_Decay = std::decay_t<decltype(active_state)>;
                if constexpr (std::is_same_v<T_Decay, BLE_Client::StateMachines::Adapter::States::off>) {
                    if(ImGui::Button("Start")) {
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
                    const float scale = GUI::Boilerplate::get_scale();
                    Spinner::Spinner("Scanning", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                }
                if constexpr (!std::is_same_v<T_Decay, BLE_Client::StateMachines::Adapter::States::off>) {
                    static bool connecting = false;
                    if(
                        shm->discovery_devices->empty() == false
                        && selected.has_value()
                        && connecting == false
                        && shm->discovery_devices->at(selected.value()).get_connected() == false
                    ) {
                        if(ImGui::Button("Connect")) {
                            connecting = true;
                            const BLE_Client::StateMachines::Connector::Events::connect connect_event { shm->discovery_devices->at(selected.value()).get_address() };
                            shm->cmd.send(connect_event);
                            std::thread([](auto shm, const BLE_Client::StateMachines::Connector::Events::connect connect_event, bool& connecting, std::vector<Windows::Client>& client_windows) {
                                for(int i = 0; i < 100; i++) {
                                    try{
                                        shm->attach_device(connect_event);
                                        client_windows.push_back(Windows::Client{std::string(connect_event.get_address()), client_windows.size(), shm });
                                        break;
                                    } catch(...) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                    }
                                }
                                connecting = false;
                            }, shm, connect_event, std::ref(connecting), std::ref(client_windows)).detach();
                        }
                    } else if(connecting == true) {
                        show_disabled_connect_button();
                        ImGui::SameLine();
                        const float scale = GUI::Boilerplate::get_scale();
                        Spinner::Spinner("ClientSpinner", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    } else {
                        show_disabled_connect_button();
                    }
                    show_table();
                }
            }, *shm->active_state);

            ImGui::End();
        }

        void BLE_Adapter::show_table() {
            if(ImGui::BeginTable("Scan", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Identifier");
                ImGui::TableNextColumn();
                ImGui::Text("Address");
                ImGui::TableNextColumn();
                ImGui::Text("Status");
                std::for_each(shm->discovery_devices->begin(), shm->discovery_devices->end(), [index = 0, this](const BLE_Client::Discovery::Device& e) mutable {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::PushID(index);
                    if(ImGui::Selectable(e.get_identifier().data(), selected.value_or(0xFFFF'FFFF'FFFF'FFFF) == index, ImGuiSelectableFlags_SpanAllColumns)) {
                        if(selected == index) {
                            selected = std::nullopt;
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
        }
    }
}