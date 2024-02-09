#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <thread>
#include <algorithm>
#include <vector>

#include <trielo/trielo.hpp>
#include "imgui_internal.h"

#include "gui/boilerplate.hpp"
#include "gui/windows/console.hpp"
#include "gui/windows/client.hpp"
#include "gui/windows/ble_client.hpp"

#include "gui/run.hpp"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

namespace GUI {
    ImGuiID top_with_dock_space(bool& done, MenuBarEnables &menu_bar_enables) {
        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("TopWindow", nullptr, window_flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiID dockspace_id = ImGui::GetID("TopDockSpace");
        if(io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
            if(ImGui::BeginMenuBar()) {
                if(ImGui::BeginMenu("File")) {
                    ImGui::MenuItem("Open");
                    ImGui::MenuItem("Save");
                    if(ImGui::MenuItem("Exit")) {
                        done = true;
                    }
                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu("Edit")) {
                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu("View")) {
                    ImGui::MenuItem("BLE Client", nullptr, &menu_bar_enables.ble_client);
                    ImGui::MenuItem("Console", nullptr, &menu_bar_enables.console);
                    ImGui::MenuItem("Calibrate", nullptr, &menu_bar_enables.calibrate);
                    ImGui::MenuItem("Measure", nullptr, &menu_bar_enables.measure);
                    ImGui::MenuItem("Debug", nullptr, &menu_bar_enables.debug);
                    ImGui::MenuItem("Auto", nullptr, &menu_bar_enables.auto_window);
                    ImGui::MenuItem("File Manager", nullptr, &menu_bar_enables.file_manager);
                    ImGui::MenuItem("Measurement Plots", nullptr, &menu_bar_enables.measurement_plots);
                    ImGui::MenuItem("Calibration Plots", nullptr, &menu_bar_enables.calibration_plots);
                    ImGui::MenuItem("Auto Plots", nullptr, &menu_bar_enables.auto_plots);
                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu("Help")) {
                    ImGui::MenuItem("About");
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }

        return dockspace_id;
    }

    DockspaceIDs split_left_center(ImGuiID dockspace_id) {
        const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace;
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);
        ImGuiID dock_id_center;
        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dock_id_center);
        return { dock_id_left, dock_id_center };
    }
}

namespace GUI {
    void run(
        bool &done,
        boost::process::child& ble_client,
        std::shared_ptr<BLE_Client::SHM::ParentSHM> shm
    ) {
        SDL_Window* window;
        SDL_Renderer* renderer;
        {
            const auto ret { Boilerplate::init() };
            window = std::get<0>(ret);
            renderer = std::get<1>(ret);
        }

        const ImVec4 clear_color { 0.45f, 0.55f, 0.60f, 1.00f };

        Boilerplate::process_events(done);
        Boilerplate::start_new_frame();
        MenuBarEnables menu_bar_enables;
        ImGuiID top_id = top_with_dock_space(done, menu_bar_enables);
        DockspaceIDs top_ids { split_left_center(top_id) };
        std::vector<Windows::Client> client_windows;
        int selected = -1;

        Windows::ble_client(menu_bar_enables.ble_client, top_ids.left, selected, shm, client_windows);
        Console console { menu_bar_enables.console };
        std::jthread stdout_reader(
            [](Console& console, boost::process::child& ble_client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
                while(ble_client.running()) {
                    const auto ret { shm->console.read_for(boost::posix_time::millisec(1)) };
                    if(ret.has_value()) {
                        console.log(ret.value());
                    }
                }
            },
            std::ref(console),
            std::ref(ble_client),
            shm
        );
        console.draw();
        Boilerplate::render(renderer, clear_color);

        while(done == false && ble_client.running()) {
            Boilerplate::process_events(done);
            Boilerplate::start_new_frame();
            top_id = top_with_dock_space(done, menu_bar_enables);

            Windows::ble_client(menu_bar_enables.ble_client, top_ids.left, selected, shm, client_windows);
            console.draw();
            for(size_t i = 0; i < client_windows.size(); i++) {
                Windows::client1(i, top_ids.center, client_windows[i], menu_bar_enables, shm);
            }

            const auto remove_it {
                std::remove_if(client_windows.begin(), client_windows.end(), [&shm](const auto& e) mutable {
                    bool ret = false;
                    if(e.enable == false) {
                        std::thread([](auto shm, auto e) {
                            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::disconnect{ e.index });
                            std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
                            shm->active_devices.erase(shm->active_devices.begin() + e.index);
                        }, shm, e).detach();
                        ret = true;
                    }
                    return ret;
                })
            };

            if(remove_it != client_windows.end()) {
                client_windows.erase(remove_it);
            }

            Boilerplate::render(renderer, clear_color);
        }
        Boilerplate::shutdown(renderer, window);

        for(size_t i = 0; i < shm->active_devices.size(); i++) {
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::disconnect{i});
        }
        shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
    }
}
