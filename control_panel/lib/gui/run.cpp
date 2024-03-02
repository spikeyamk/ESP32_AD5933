#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <thread>
#include <algorithm>
#include <vector>
#include <string_view>

#include <trielo/trielo.hpp>
#include <SDL3/SDL_events.h>
#include "imgui_internal.h"
#include "implot.h"

#include "gui/boilerplate.hpp"
#include "gui/top.hpp"
#include "gui/windows/console.hpp"
#include "gui/windows/ble_adapter.hpp"
#include "gui/windows/client/client.hpp"

#include "gui/run.hpp"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

namespace GUI {
    DockspaceIDs split_left_center(ImGuiID dockspace_id) {
        const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace;
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
        boost::process::child& ble_client,
        std::shared_ptr<BLE_Client::SHM::ParentSHM> shm
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

        Boilerplate::process_events(done, window, renderer);
        Boilerplate::start_new_frame();
        Top top { settings_file };
        ImGuiID top_id = top.draw(done);
        DockspaceIDs top_ids { split_left_center(top_id) };
        std::vector<Windows::Client> client_windows;

        Windows::BLE_Adapter ble_connector { shm, client_windows };
        ble_connector.draw(top.menu_bar_enables.ble_adapter, top_ids.left);
        Windows::Console console { top.menu_bar_enables.console };
        std::jthread stdout_reader(
            [](Windows::Console& console, boost::process::child& ble_client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
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
        Boilerplate::render(renderer);

        while(done == false && ble_client.running()) {
            Boilerplate::process_events(done, window, renderer);
            Boilerplate::start_new_frame();
            top_id = top.draw(done);
            
            ble_connector.draw(top.menu_bar_enables.ble_adapter, top_ids.left);
            console.draw();
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

            if(top.menu_bar_enables.demo) {
                ImGui::ShowDemoWindow();
            }

            Boilerplate::render(renderer);
        }
        Boilerplate::shutdown(renderer, window);

        if(ble_client.running() == false) {
            std::cout << "ERROR: GUI::Windows::run: ble_client exited before gui\n";
        }

        for(size_t i = 0; i < shm->active_devices.size(); i++) {
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::disconnect{i});
        }
        shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
    }
}
