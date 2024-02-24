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
#include "gui/windows/ble_adapter.hpp"
#include "gui/windows/console.hpp"
#include "gui/windows/client/client.hpp"
#include "imgui_custom/markdown.hpp"

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
        static bool settings_clicked { false };
        static bool about_clicked { false };
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
                    if(ImGui::MenuItem("Settings")) {
                        settings_clicked = true;
                    }
                    ImGui::EndMenu();
                }


                if(ImGui::BeginMenu("View")) {
                    ImGui::MenuItem((const char*) Windows::BLE_Adapter::name.data(), nullptr, &menu_bar_enables.ble_adapter);
                    ImGui::MenuItem((const char*) Windows::Console::name.data(), nullptr, &menu_bar_enables.console);
                    ImGui::MenuItem((const char*) Windows::Calibrate::name_base.data(), nullptr, &menu_bar_enables.calibrate);
                    ImGui::MenuItem((const char*) Windows::Measure::name_base.data(), nullptr, &menu_bar_enables.measure);
                    ImGui::MenuItem((const char*) Windows::Debug::name_base.data(), nullptr, &menu_bar_enables.debug);
                    ImGui::MenuItem((const char*) Windows::Auto::name_base.data(), nullptr, &menu_bar_enables.auto_window);
                    ImGui::MenuItem((const char*) Windows::FileManager::name_base.data(), nullptr, &menu_bar_enables.file_manager);
                    ImGui::MenuItem((const char*) Windows::Plots::Measurement::name_base.data(), nullptr, &menu_bar_enables.measurement_plots);
                    ImGui::MenuItem((const char*) Windows::Plots::Calibration::name_base.data(), nullptr, &menu_bar_enables.calibration_plots);
                    ImGui::MenuItem((const char*) Windows::Plots::Auto::name_base.data(), nullptr, &menu_bar_enables.auto_plots);
                    ImGui::MenuItem("Demo", nullptr, &menu_bar_enables.demo);
                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu("Help")) {
                    if(ImGui::MenuItem("About")) {
                        about_clicked = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            if(settings_clicked) {
                ImGui::OpenPopup("Settings");
                // Always center this window when appearing
                const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                settings_clicked = false;
            }

            if(ImGui::BeginPopupModal("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                const auto show_theme_combo = []() {
                    static int theme_combo { Boilerplate::respect_system_theme ? 0 : 1 };
                    if(ImGui::Combo("Theme", &theme_combo, "Respect Global System Settings\0Dark\0Light\0Classic\0")) {
                        Boilerplate::respect_system_theme = (theme_combo == 0 ? true : false);
                        switch(theme_combo) {
                            case 0:
                                SDL_Event event;
                                SDL_zero(event);
                                event.type = SDL_EVENT_SYSTEM_THEME_CHANGED;
                                SDL_PushEvent(&event);
                                break;
                            case 1:
                                ImGui::StyleColorsDark();
                                break;
                            case 2:
                                ImGui::StyleColorsLight();
                                break;
                            case 3:
                                ImGui::StyleColorsClassic();
                                break;
                        }
                    }
                };

                const auto show_scale_combo = []() {
                    static constexpr float scales[] {
                        0.50f,
                        0.75f,
                        1.00f,
                        1.25f,
                        1.50f,
                        1.75f,
                        2.00f,
                        2.25f,
                        2.50f,
                        2.75f,
                        3.00f,
                    };

                    static constexpr std::string_view scale_inputs {
                        "Respect Global System Settings\0"
                        "50%\0"
                        "75%\0"
                        "100%\0"
                        "125%\0"
                        "150%\0"
                        "175%\0"
                        "200%\0"
                        "225%\0"
                        "250%\0"
                        "275%\0"
                        "300%\0"
                    };

                    static int scale_combo { Boilerplate::respect_system_scale ? 0 : 3 };
                    if(ImGui::Combo("Scale", &scale_combo, scale_inputs.data())) {
                        Boilerplate::respect_system_scale = (scale_combo == 0 ? true : false);
                        SDL_Event event;
                        SDL_zero(event);
                        if(scale_combo != 0) {
                            event.type = Boilerplate::event_user_scale_event_type;
                            event.user.code = *reinterpret_cast<const Uint32*>(scales + static_cast<size_t>(scale_combo - 1));
                            SDL_PushEvent(&event);
                        } else {
                            event.type = SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED;
                            SDL_PushEvent(&event);
                        }
                    }
                };

                show_theme_combo();
                show_scale_combo();

                ImGui::Separator();
                ImGui::Checkbox("Local Time", &ImPlot::GetStyle().UseLocalTime);
                ImGui::SameLine();
                ImGui::Checkbox("ISO 8601", &ImPlot::GetStyle().UseISO8601);
                ImGui::SameLine();
                ImGui::Checkbox("24 Hour Clock", &ImPlot::GetStyle().Use24HourClock);
                ImGui::Separator();

                if(ImGui::Button("OK", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if(about_clicked) {
                ImGui::OpenPopup("About");
                // Always center this window when appearing
                const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                about_clicked = false;
            }

            if(ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text((const char*)
                    u8"Tu bude naša krásna nádherná about page ani neviem čo tu vôbec budem dávať.\n"
                    u8"Tu budu asi externé závislosti? A ich licencie? To asi ani robiť nebudem."
                );

                static const std::string_view page_link { "Repo: [https://www.github.com/spikeyamk/ESP32_AD5933](https://www.github.com/spikeyamk/ESP32_AD5933)" };
                ImGui::RenderMarkdown(page_link.data(), page_link.size());

                if(ImGui::Button("OK", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
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
        const ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dock_id_center);
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

        Boilerplate::process_events(done, window, renderer);
        Boilerplate::start_new_frame();
        MenuBarEnables menu_bar_enables;
        ImGuiID top_id = top_with_dock_space(done, menu_bar_enables);
        DockspaceIDs top_ids { split_left_center(top_id) };
        std::vector<Windows::Client> client_windows;

        Windows::BLE_Adapter ble_connector { shm, client_windows };
        ble_connector.draw(menu_bar_enables.ble_adapter, top_ids.left);
        Windows::Console console { menu_bar_enables.console };
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
        Boilerplate::render(renderer, clear_color);

        while(done == false && ble_client.running()) {
            Boilerplate::process_events(done, window, renderer);
            Boilerplate::start_new_frame();
            top_id = top_with_dock_space(done, menu_bar_enables);
            
            ble_connector.draw(menu_bar_enables.ble_adapter, top_ids.left);
            console.draw();
            for(size_t i = 0; i < client_windows.size(); i++) {
                client_windows[i].draw(top_ids.center, menu_bar_enables);
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

            if(menu_bar_enables.demo) {
                ImGui::ShowDemoWindow();
            }

            Boilerplate::render(renderer, clear_color);
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
