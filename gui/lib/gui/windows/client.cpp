#include "imgui_internal.h"

#include "gui/windows/debug_registers.hpp"
#include "gui/windows/sweep.hpp"
#include "gui/windows/plots/measurement.hpp"
#include "gui/windows/plots/calibration.hpp"

#include "gui/windows/client.hpp"

namespace GUI {
    namespace Windows {
        void client1(const int i, ImGuiID center_id, Client &client, MenuBarEnables &enables, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
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
                    measurement_plots(right_id, enables.measurement_plots, client);
                    calibration_plots(i, right_id, enables.calibration_plots, client);
                    sweep(i, dockspace_id, enables.configure, client, shm);
                    debug_registers(i, dockspace_id, enables.debug_registers, client, shm);
                    first++;
                    ImGui::DockBuilderFinish(dockspace_id);
                } else {
                    if(enables.measurement_plots) {
                        measurement_plots(ImGui::GetID(static_cast<void*>(nullptr)), enables.measurement_plots, client);
                    }
                    if(enables.calibration_plots) {
                        calibration_plots(i, ImGui::GetID(static_cast<void*>(nullptr)), enables.calibration_plots, client);
                    }
                    if(enables.configure) {
                        sweep(i, ImGui::GetID(static_cast<void*>(nullptr)), enables.configure, client, shm);
                    }
                    if(enables.debug_registers) {
                        debug_registers(i, ImGui::GetID(static_cast<void*>(nullptr)), enables.debug_registers, client, shm);
                    }
                }
            }
            ImGui::End();
        }
    }
}