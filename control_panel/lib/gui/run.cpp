#include <thread>

#include <trielo/trielo.hpp>
#include <imgui_internal.h>
#include <implot.h>

#include "gui/boilerplate.hpp"
#include "gui/top.hpp"
#include "gui/windows/ble_adapter.hpp"
#include "gui/windows/client/client.hpp"
#include "gui/windows/implot_dense_test.hpp"

#include "gui/run.hpp"

namespace GUI {
    DockspaceIDs split_left_center(ImGuiID dockspace_id) {
        const ImGuiDockNodeFlags dockspace_flags { ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace };
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);
        ImGuiID dock_id_center;
        const ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.225f, nullptr, &dock_id_center);
        return { dock_id_left, dock_id_center };
    }

    void draw_quit_popup(bool& sdl_event_quit, bool& done, const auto& client_windows) {
        if(sdl_event_quit) {
            if(client_windows.empty()) {
                done = true;
            } else {
                ImGui::OpenPopup("Quit");
                ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            }
            sdl_event_quit = false;
        }

        if(ImGui::BeginPopupModal("Quit")) {
            ImGui::Text("Some connections are still open.\nAre you sure you want to quit?");
            if(ImGui::Button("OK", ImVec2(64.0f * Boilerplate::get_scale(), 0.0f))) {
                ImGui::CloseCurrentPopup();
                done = true;
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel", ImVec2(64.0f * Boilerplate::get_scale(), 0.0f))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

namespace GUI {
    void run(
        bool &done,
        std::shared_ptr<BLE_Client::SHM::SHM> shm
    ) {
        SDL_Window* window { nullptr };
        SDL_Renderer* renderer { nullptr };
        ns::SettingsFile settings_file;
        {
            const auto ret { Boilerplate::init() };
            window = std::get<0>(ret);
            renderer = std::get<1>(ret);
            settings_file = std::get<2>(ret);
        }

        if(window == nullptr || renderer == nullptr) {
            return;
        }

        bool sdl_event_quit { false };
        Boilerplate::process_events(window, renderer, sdl_event_quit);
        Boilerplate::start_new_frame();

        Top top { settings_file };
        bool reload { false };
        ImGuiID top_id = top.draw(done, reload);
        DockspaceIDs top_ids { split_left_center(top_id) };
        std::vector<Windows::Client> client_windows;
        Windows::PopupQueue popup_queue {};

        Windows::BLE_Adapter ble_adapter { shm, client_windows, popup_queue };
        ble_adapter.draw(top.menu_bar_enables.ble_adapter, top_ids.left);
        Boilerplate::render(renderer);

        bool reloaded { false };
        while(done == false) {
            Boilerplate::process_events(window, renderer, sdl_event_quit);

            Boilerplate::start_new_frame();
            draw_quit_popup(sdl_event_quit, done, client_windows);
            top_id = top.draw(done, reload);
            
            ble_adapter.draw(top.menu_bar_enables.ble_adapter, top_ids.left);
            for(size_t i = 0; i < client_windows.size(); i++) {
                client_windows[i].draw(top_ids.center, top.menu_bar_enables);
            }

            const auto remove_it {
                std::remove_if(client_windows.begin(), client_windows.end(), [&shm](const Windows::Client& e) {
                    if(e.enable == true) {
                        return false;
                    }

                    std::thread([](auto shm, const size_t index) {
                        shm->cmd.send(BLE_Client::StateMachines::Connection::Events::disconnect{ index });
                        std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
                        shm->active_devices.erase(shm->active_devices.begin() + index);
                    }, shm, e.index).detach();
                    return true;
                })
            };

            if(remove_it != client_windows.end()) {
                client_windows.erase(remove_it);
            }

            if(top.menu_bar_enables.imgui_demo) {
                ImGui::ShowDemoWindow();
            }

            if(top.menu_bar_enables.implot_demo) {
                ImPlot::ShowDemoWindow();
            }

            if(popup_queue.empty() == false && popup_queue.active() == false) {
                popup_queue.activate_front();
            }

            if(popup_queue.active()) {
                popup_queue.show_active();
            }

            if(top.menu_bar_enables.implot_dense_test) {
                ImPlotDenseTest::draw(top.menu_bar_enables.implot_dense_test);
            }

            if(reloaded) {
                reloaded = false;
                Boilerplate::render_skip_frame(renderer);
            } else {
                Boilerplate::render(renderer);
            }
            
            if(reload) {
                reload = false;
                Boilerplate::reload(window, renderer);
                reloaded = true;
            }
        }
        Boilerplate::shutdown(renderer, window);

        for(size_t i = 0; i < shm->active_devices.size(); i++) {
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::disconnect{i});
        }
        shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
    }
}
