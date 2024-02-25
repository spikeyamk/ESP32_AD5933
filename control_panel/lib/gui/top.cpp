#include "imgui_internal.h"
#include "implot.h"

#include "gui/boilerplate.hpp"
#include "imgui_custom/markdown.hpp"
#include "gui/windows/console.hpp"
#include "gui/windows/client/client.hpp"
#include "gui/windows/ble_adapter.hpp"

#include "gui/top.hpp"

namespace GUI {
    Top::Top() {
        try {
            std::ifstream file(ns::settings_file_path);
            if(file.is_open() == false) {
                return;
            }

            json j;
            file >> j;
            const ns::SettingsFile settings_file = j;
            theme_combo = settings_file.settings.theme_combo;
            scale_combo = settings_file.settings.scale_combo;
            update_theme();
            update_scale();
            ImPlot::GetStyle().UseLocalTime = settings_file.settings.local_time;
            ImPlot::GetStyle().UseISO8601 = settings_file.settings.iso_8601;
            ImPlot::GetStyle().Use24HourClock = settings_file.settings.twenty_four_hour_clock;
        } catch(const std::exception& e) {
            std::cout << "ERROR: GUI::Top::Top(): error loading " << ns::settings_file_path << " file and parsing to json: exception: " << e.what() << std::endl;
        }
    }

    ImGuiID Top::draw(bool& done) {
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
                draw_settings();
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
                draw_about_page();
                ImGui::EndPopup();
            }

            ImGui::End();
        }

        return dockspace_id;
    }

    void Top::draw_about_page() {
        ImGui::Text((const char*)
            u8"Tu bude naša krásna nádherná about page ani neviem čo tu vôbec budem dávať.\n"
            u8"Tu budu asi externé závislosti? A ich licencie? To asi ani robiť nebudem."
        );

        static const std::string_view page_link { "Repo: [https://www.github.com/spikeyamk/ESP32_AD5933](https://www.github.com/spikeyamk/ESP32_AD5933)" };
        ImGui::RenderMarkdown(page_link.data(), page_link.size());

        if(ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
    }

    void Top::draw_settings() {
        if(ImGui::Combo("Theme", &theme_combo,
            "Respect Global System Settings\0"
            "Dark\0"
            "Light\0"
            "Classic\0")
        ) {
            update_theme();
        }

        if(ImGui::Combo("Scale", &scale_combo, Scales::identifiers.data())) {
            update_scale();
        }

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

        ImGui::SameLine();

        if(ImGui::Button("Save", ImVec2(120, 0))) {
            save_settings();
            ImGui::CloseCurrentPopup();
        }
    }

    void Top::save_settings() {
        try {
            std::ofstream file(ns::settings_file_path);
            if(file.is_open() == false) {
                std::cout << "ERROR: GUI:Top::save_settings: could not open " << ns::settings_file_path << " file for writing\n";
                return;
            }

            const ns::SettingsFile settings_file {
                ns::Settings {
                    .theme_combo = theme_combo,
                    .scale_combo = scale_combo,
                    .local_time = ImPlot::GetStyle().UseLocalTime,
                    .iso_8601 = ImPlot::GetStyle().UseISO8601,
                    .twenty_four_hour_clock = ImPlot::GetStyle().Use24HourClock,
                }
            };
            const json j = settings_file;
            file << std::setw(4) << j;
        } catch(const std::exception& e) {
            std::cout << "ERROR: GUI:Top::save_settings: failed: exception: " << e.what() << std::endl;
        }
    }

    void Top::update_theme() {
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

    void Top::update_scale() {
        Boilerplate::respect_system_scale = (scale_combo == 0 ? true : false);
        SDL_Event event;
        SDL_zero(event);
        if(scale_combo != 0) {
            event.type = Boilerplate::event_user_scale_event_type;
            event.user.code = *reinterpret_cast<const Uint32*>(Scales::values + static_cast<size_t>(scale_combo - 1));
            SDL_PushEvent(&event);
        } else {
            event.type = SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED;
            SDL_PushEvent(&event);
        }
    }
}