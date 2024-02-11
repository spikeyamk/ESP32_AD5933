#include "imgui_internal.h"

#include "gui/windows/debug.hpp"
#include "gui/windows/calibrate.hpp"
#include "gui/windows/measure.hpp"
#include "gui/windows/file_manager.hpp"
#include "gui/windows/plots/measurement.hpp"
#include "gui/windows/plots/calibration.hpp"

#include "gui/windows/client.hpp"
namespace GUI {
    namespace Windows {
        Client::Client(const std::string name, const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> parent_shm) :
            name{ name },
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

        void client1(const size_t i, ImGuiID center_id, Client &client, MenuBarEnables &enables, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
            if(client.enable == false) {
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

            std::string name {
                client.name
                + std::string("##")
                + std::to_string(i)
            };

            static int first = 0;
            if(first == i) {
                ImGui::DockBuilderDockWindow(name.c_str(), center_id);
            }

            ImGui::Begin(name.c_str(), &(client.enable), window_flags);

            if(client.enable == false) {
                ImGui::End();
                return;
            }

            ImGuiIO& io = ImGui::GetIO();
            if(io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
                std::string dockspace_name {
                    std::string("DockSpace")
                    + client.name
                    + std::string("##")
                    + std::to_string(i)
                };
                ImGuiID dockspace_id = ImGui::GetID(dockspace_name.c_str());
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
                if(first == i) {
                    static const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace;
                    ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
                    ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
                    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());
                    ImGuiID right_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);
                    client.measurement_plots_window.draw(enables.measurement_plots, right_id);
                    client.calibration_plots_window.draw(enables.calibration_plots, right_id);
                    client.auto_plots_window.draw(enables.auto_plots, right_id);
                    client.debug_window.draw(enables.debug, dockspace_id);
                    client.calibrate_window.draw(enables.calibrate, dockspace_id);
                    client.measure_window.draw(enables.measure, dockspace_id);
                    client.file_manager_window.draw(enables.file_manager, dockspace_id);
                    client.auto_window.draw(enables.auto_window, dockspace_id);
                    first++;
                    ImGui::DockBuilderFinish(dockspace_id);
                } else {
                    if(enables.measurement_plots) {
                        if(client.measure_window.single_plotted == false) {
                            client.measurement_plots_window.update_single_vectors(client.measure_window.single_vectors.freq_float, client.measure_window.single_vectors.raw_measurement, client.measure_window.single_vectors.measurement);
                            client.measure_window.single_plotted = true;
                        }
                        if(client.measure_window.periodic_vectors.periodic_points.empty() == false) {
                            client.measurement_plots_window.update_periodic_vectors(client.measure_window.periodic_vectors.freq_float, client.measure_window.periodic_vectors.periodic_points);
                        }
                        client.measurement_plots_window.draw(enables.measurement_plots, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.calibration_plots) {
                        if(client.calibrate_window.plotted == false) {
                            client.calibration_plots_window.update_vectors(client.calibrate_window.config, client.calibrate_window.raw_calibration, client.calibrate_window.calibration);
                            client.calibrate_window.plotted = true;
                        }
                        client.calibration_plots_window.draw(enables.calibration_plots, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.auto_plots) {
                        if(client.auto_window.send_points.empty() == false) {
                            client.auto_plots_window.update_send_vectors(client.auto_window.send_points);
                        }
                        client.auto_plots_window.draw(enables.auto_plots, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.debug) {
                        client.debug_window.draw(enables.debug, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.calibrate) {
                        client.calibrate_window.draw(enables.calibrate, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.measure) {
                        client.measure_window.draw(enables.measure, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.file_manager) {
                        client.file_manager_window.draw(enables.file_manager, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                    if(enables.auto_window) {
                        client.auto_window.draw(enables.auto_window, ImGui::GetID(static_cast<void*>(nullptr)));
                    }
                }
            }
            ImGui::End();
        }
    }
}
