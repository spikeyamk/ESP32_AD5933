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
    DockspaceIDs split_left_center(const ImGuiID dockspace_id) {
        const ImGuiDockNodeFlags dockspace_flags { ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace };
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);
        ImGuiID dock_id_center;
        const ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.225f, nullptr, &dock_id_center);
        return { dock_id_left, dock_id_center };
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

        bool event_quit { false };
        Boilerplate::process_events(window, renderer, event_quit);
        Boilerplate::start_new_frame();

        Windows::PopupQueue popup_queue {};
        Top top { settings_file};
        const ImGuiID top_id { top.draw(event_quit) };
        const DockspaceIDs top_ids { split_left_center(top_id) };
        std::vector<Windows::Client> client_windows;

        Windows::BLE_Adapter ble_adapter { shm, client_windows, popup_queue };
        ble_adapter.draw(top.menu_bar_enables.ble_adapter, top_ids.left);
        Boilerplate::render(renderer);

        while(done == false) {
            Boilerplate::process_events(window, renderer, event_quit);
            Boilerplate::start_new_frame();
            top.draw(event_quit);

            if(event_quit) {
                if(client_windows.empty()) {
                    done = true;
                } else {
                    popup_queue.push_back(
                        "Quit",
                        "",
                        [&done, &popup_queue]() {
                            ImGui::Text("Some connections are still open.\nAre you sure you want to quit?");
                            if(ImGui::Button("OK", ImVec2(64.0f * Boilerplate::get_scale(), 0.0f))) {
                                done = true;
                                popup_queue.deactivate();
                            }
                            ImGui::SameLine();
                            if(ImGui::Button("Cancel", ImVec2(64.0f * Boilerplate::get_scale(), 0.0f))) {
                                popup_queue.deactivate();
                            }
                        }
                    );
                }
                event_quit = false;
            }
            
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
                        shm->active_devices->erase(shm->active_devices->begin() + index);
                    }, shm, e.index).detach();
                    return true;
                })
            };

            if(remove_it != client_windows.end()) {
                client_windows.erase(remove_it);
            }

            if(top.menu_bar_enables.imgui_demo) {
                ImGui::ShowDemoWindow(&top.menu_bar_enables.imgui_demo);
            }

            if(top.menu_bar_enables.implot_demo) {
                ImPlot::ShowDemoWindow(&top.menu_bar_enables.implot_demo);
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

            Boilerplate::render(renderer);
        }
        Boilerplate::shutdown(renderer, window);

        for(size_t i = 0; i < shm->active_devices->size(); i++) {
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::disconnect{i});
        }
        shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
    }
}
