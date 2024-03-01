#include <thread>

#include "imgui_internal.h"

#include "imgui_custom/spinner.hpp"
#include "gui/boilerplate.hpp"
#include "misc/variant_tester.hpp"

#include "gui/windows/ble_adapter.hpp"

namespace GUI {
    namespace Windows {
        BLE_Adapter::BLE_Adapter(std::shared_ptr<BLE_Client::SHM::ParentSHM> shm, std::vector<Windows::Client>& client_windows) :
            shm { shm },
            client_windows { client_windows }
        {
            shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::turn_on{});
            validate_start_attempt();
            shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::start_discovery{});
            std::jthread checker([](std::stop_token st, BLE_Adapter& self) {
                bool first { true };
                while(st.stop_requested() == false) {
                    if(first) {
                        if(variant_tester<BLE_Client::StateMachines::Adapter::States::off>(*self.shm->active_state) == false) {
                            first = false;
                        }
                    } else {
                        if(variant_tester<BLE_Client::StateMachines::Adapter::States::off>(*self.shm->active_state)) {
                            first = true;
                            self.show_ble_off_error_pop_up = true;
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }, std::ref(*this));
            checker.detach();
            stop_sources.push_back(checker.get_stop_source());
        }

        BLE_Adapter::~BLE_Adapter() {
            for(auto& e: stop_sources) {
                e.request_stop();
            }
        }

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

            std::visit([&](auto&& active_state) {
                using T_Decay = std::decay_t<decltype(active_state)>;
                if constexpr (std::is_same_v<T_Decay, BLE_Client::StateMachines::Adapter::States::off>) {
                    if(ImGui::Button("Retry")) {
                        shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::turn_on{});
                        validate_start_attempt();
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
                        && [&]() {
                            if(std::find_if(client_windows.begin(), client_windows.end(), [&](const auto& e) {
                                return e.get_address() == shm->discovery_devices->at(selected.value()).get_address();
                            }) != client_windows.end()) {
                                return false;
                            }

                            return true;
                        }()
                    ) {
                        if(ImGui::Button("Connect")) {
                            connecting = true;
                            const BLE_Client::StateMachines::Connector::Events::connect connect_event { shm->discovery_devices->at(selected.value()).get_address() };
                            shm->cmd.send(connect_event);
                            std::jthread t1([](std::stop_token st, auto shm, const BLE_Client::StateMachines::Connector::Events::connect connect_event, bool& connecting, std::vector<Windows::Client>& client_windows, bool& show_connection_attempt_timeout_error_pop_up) {
                                for(size_t i = 0, timeout_ms = 5'000; i < timeout_ms; i++) {
                                    if(st.stop_requested()) {
                                        return;
                                    }
                                    try{
                                        shm->attach_device(connect_event);
                                        client_windows.push_back(std::move(Windows::Client{std::string(connect_event.get_address()), client_windows.size(), shm }));
                                        connecting = false;
                                        return;
                                    } catch(...) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                                    }
                                }
                                show_connection_attempt_timeout_error_pop_up = true;
                                connecting = false;
                            }, shm, connect_event, std::ref(connecting), std::ref(client_windows), std::ref(show_connection_attempt_timeout_error_pop_up));
                            t1.detach();
                            stop_sources.push_back(t1.get_stop_source());
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

            if(show_ble_off_error_pop_up) {
                ImGui::OpenPopup("Error##0");
                // Always center this window when appearing
                ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                show_ble_off_error_pop_up = false;
            }

            if(ImGui::BeginPopupModal("Error##0", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Bluetooth is turned off.");
                if(ImGui::Button("OK")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if(show_connection_attempt_timeout_error_pop_up) {
                ImGui::OpenPopup("Error##1");
                // Always center this window when appearing
                ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                show_connection_attempt_timeout_error_pop_up = false;
            }

            if(ImGui::BeginPopupModal("Error##1", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Connection attempt timed out");
                if(ImGui::Button("OK")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
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
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.1f, 1.0f), "Connected");
                    } else {
                        ImGui::Text("Disconnected");
                    }
                    index++;  
                });
                ImGui::EndTable();
            }
        }

        void BLE_Adapter::show_disabled_connect_button() const {
            ImGui::BeginDisabled();
            ImGui::Button("Connect");
            ImGui::EndDisabled();
        }

        void BLE_Adapter::validate_start_attempt() {
            std::jthread t1([](std::stop_token st, BLE_Adapter& self) {
                for(size_t i = 0, timeout = 200; i < timeout; i++) {
                    if(st.stop_requested()) {
                        return;
                    }
                    if(variant_tester<BLE_Client::StateMachines::Adapter::States::off>(*self.shm->active_state) == false) {
                        return;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                self.show_ble_off_error_pop_up = true;
            }, std::ref(*this));
            t1.detach();
            stop_sources.push_back(t1.get_stop_source());
        }
    }
}