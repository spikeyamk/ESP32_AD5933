#include <thread>

#include "imgui_internal.h"

#include "gui/boilerplate.hpp"

#include "gui/windows/client/client.hpp"

namespace GUI {
    namespace Windows {
        Client::Client(const std::string name, const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> parent_shm) :
            address{ name },
            name{ std::string(name).append("##").append(std::to_string(index)) },
            dockspace_name { std::string("DockSpace").append(name).append("##").append(std::to_string(index)) },
            index{ index },
            shm{ parent_shm },
            calibrate_window { index, parent_shm },
            calibration_plots_window { index },
            measure_window { index, parent_shm },
            measurement_plots_window { index },
            debug_window { index, parent_shm },
            auto_window { index, parent_shm },
            auto_plots_window { index }
        {}

        const std::string& Client::get_address() const {
            return address;
        }

        void Client::draw(const ImGuiID center_id, Top::MenuBarEnables &enables) {
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

            if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
                ImGuiID dockspace_id = ImGui::GetID(dockspace_name.c_str());
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
                if(first) {
                    static const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace;
                    ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
                    ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
                    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());
                    const ImGuiID right_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.5f, nullptr, &dockspace_id);
                    calibration_plots_window.draw(enables.calibration_plots, right_id);
                    measurement_plots_window.draw(enables.measurement_plots, right_id);
                    auto_plots_window.draw(enables.auto_plots, right_id);
                    auto_window.draw(enables.auto_window, dockspace_id, lock);
                    measure_window.draw(enables.measure, dockspace_id, lock);
                    calibrate_window.draw(enables.calibrate, dockspace_id, lock);
                    debug_window.draw(enables.debug, dockspace_id, lock);
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
                        if(auto_window.save_points.empty() == false) {
                            auto_plots_window.update_save_vectors(auto_window.save_points);
                        }
                        auto_plots_window.draw(enables.auto_plots, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.debug) {
                        debug_window.draw(enables.debug, ImGui::GetID(static_cast<void*>(nullptr)), lock);
                    }
                    if(enables.calibrate) {
                        calibrate_window.draw(enables.calibrate, ImGui::GetID(static_cast<void*>(nullptr)), lock);
                    }
                    if(enables.measure) {
                        if(calibrate_window.calibration_queue_to_load_into_measurement.empty() == false) {
                            measure_window.load_from_memory(calibrate_window.calibration_queue_to_load_into_measurement.front());
                            calibrate_window.calibration_queue_to_load_into_measurement.pop();
                        }
                        measure_window.draw(enables.measure, ImGui::GetID(static_cast<void*>(nullptr)), lock);
                    }
                    if(enables.auto_window) {
                        auto_window.draw(enables.auto_window, ImGui::GetID(static_cast<void*>(nullptr)), lock);
                    }
                }
            }
            ImGui::End();

            auto it { std::find_if(shm->discovery_devices->begin(), shm->discovery_devices->end(), [&](const auto& e) {
                return e.get_address() == address;
            }) };

            if(it != shm->discovery_devices->end()) {
                if(it->get_connected() == false && lock != Lock::UnexpectedDisconnection ) {
                    ImGui::OpenPopup("Error##2");
                    // Always center this window when appearing
                    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                    lock = Lock::UnexpectedDisconnection;
                }
            }

            if(ImGui::BeginPopupModal("Error##2", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("%s Unexpectedly disconnected", address.c_str());
                if(ImGui::Button("OK")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }
}
