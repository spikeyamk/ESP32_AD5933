#include <thread>

#include "imgui_internal.h"

#include "gui/windows/debug.hpp"
#include "gui/windows/calibrate.hpp"
#include "gui/windows/measure.hpp"
#include "gui/windows/file_manager.hpp"
#include "gui/windows/plots/measurement.hpp"
#include "gui/windows/plots/calibration.hpp"
#include "imgui_custom/spinner.hpp"
#include "gui/boilerplate.hpp"

#include "gui/windows/client.hpp"

namespace GUI {
    namespace Windows {
        BLE_Connector::BLE_Connector(std::shared_ptr<BLE_Client::SHM::ParentSHM> shm, std::vector<Windows::Client>& client_windows) :
            shm { shm },
            client_windows { client_windows }
        {}

        void BLE_Connector::draw(bool &enable, const ImGuiID left_id) {
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
                            std::thread([](auto shm, const BLE_Client::StateMachines::Connector::Events::connect connect_event, bool& connecting, std::vector<Windows::Client>& client_windows, BLE_Client::Discovery::Device& device) {
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
                            }, shm, connect_event, std::ref(connecting), std::ref(client_windows), std::ref(shm->discovery_devices->at(selected.value()))).detach();
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

        void BLE_Connector::show_table() {
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

namespace GUI {
    namespace Windows {
        Client::Client(const std::string name, const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> parent_shm) :
            name{ std::string(name).append("##").append(std::to_string(index)) },
            dockspace_name { std::string("DockSpace").append(name).append("##").append(std::to_string(index)) },
            index{ index },
            calibrate_window { index, parent_shm },
            calibration_plots_window { index },
            measure_window { index, parent_shm },
            measurement_plots_window { index },
            file_manager_window { index, parent_shm },
            debug_window { index, parent_shm },
            auto_window { index, parent_shm },
            auto_plots_window { index }
        {}

        void Client::draw(const ImGuiID center_id, MenuBarEnables &enables) {
            if(enable == false) {
                return;
            }

            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove;

            //window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            //window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
            // and handle the pass-thru hole, so we ask Begin() to not render a background.
            if(dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

            if(first) {
                ImGui::DockBuilderDockWindow(name.c_str(), center_id);
            }

            ImGui::Begin(name.c_str(), &(enable), window_flags);

            if(enable == false) {
                ImGui::End();
                return;
            }

            ImGuiIO& io = ImGui::GetIO();
            if(io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
                ImGuiID dockspace_id = ImGui::GetID(dockspace_name.c_str());
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
                if(first) {
                    static const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace;
                    ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
                    ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
                    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());
                    ImGuiID right_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);
                    measurement_plots_window.draw(enables.measurement_plots, right_id);
                    calibration_plots_window.draw(enables.calibration_plots, right_id);
                    auto_plots_window.draw(enables.auto_plots, right_id);
                    debug_window.draw(enables.debug, dockspace_id);
                    calibrate_window.draw(enables.calibrate, dockspace_id);
                    measure_window.draw(enables.measure, dockspace_id);
                    file_manager_window.draw(enables.file_manager, dockspace_id);
                    auto_window.draw(enables.auto_window, dockspace_id);
                    ImGui::DockBuilderFinish(dockspace_id);
                    first = false;
                } else {
                    if(enables.measurement_plots) {
                        if(measure_window.single_plotted == false) {
                            measurement_plots_window.update_single_vectors(measure_window.single_vectors.freq_float, measure_window.single_vectors.raw_measurement, measure_window.single_vectors.measurement);
                            measure_window.single_plotted = true;
                        }
                        if(measure_window.periodic_vectors.periodic_points.empty() == false) {
                            measurement_plots_window.update_periodic_vectors(measure_window.periodic_vectors.freq_float, measure_window.periodic_vectors.periodic_points);
                        }
                        measurement_plots_window.draw(enables.measurement_plots, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.calibration_plots) {
                        if(calibrate_window.plotted == false) {
                            calibration_plots_window.update_vectors(calibrate_window.config, calibrate_window.raw_calibration, calibrate_window.calibration);
                            calibrate_window.plotted = true;
                        }
                        calibration_plots_window.draw(enables.calibration_plots, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.auto_plots) {
                        if(auto_window.send_points.empty() == false) {
                            auto_plots_window.update_send_vectors(auto_window.send_points);
                        }
                        auto_plots_window.draw(enables.auto_plots, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.debug) {
                        debug_window.draw(enables.debug, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.calibrate) {
                        calibrate_window.draw(enables.calibrate, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.measure) {
                        measure_window.draw(enables.measure, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.file_manager) {
                        file_manager_window.draw(enables.file_manager, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.auto_window) {
                        auto_window.draw(enables.auto_window, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                }
            }
            ImGui::End();
        }
    }
}
