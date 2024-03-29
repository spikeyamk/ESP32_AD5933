#include <trielo/trielo.hpp>
#include "imgui_internal.h"
#include "implot.h"

#include "gui/boilerplate.hpp"
#include "imgui_custom/markdown.hpp"
#include "gui/windows/console.hpp"
#include "gui/windows/client/client.hpp"
#include "gui/windows/ble_adapter.hpp"
#include "legal/gpl2.hpp"
#include "legal/boost.hpp"
#include "legal/dear_imgui.hpp"
#include "legal/implot.hpp"
#include "legal/fmt.hpp"
#include "legal/json.hpp"
#include "legal/nativefiledialog_extended.hpp"
#include "legal/sdl3.hpp"
#include "legal/semver.hpp"
#include "legal/simpleble.hpp"
#include "legal/sml.hpp"
#include "legal/ubuntu_sans_fonts.hpp"
#include "legal/utfconv.hpp"

#include "gui/top.hpp"

namespace GUI {
    Top::Top(const ns::SettingsFile& settings_file) :
        theme_combo { settings_file.settings.theme_combo },
        scale_combo { settings_file.settings.scale_combo }
    {}

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
        if(io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
            if(ImGui::BeginMenuBar()) {
                if(ImGui::BeginMenu("File")) {
                    ImGui::MenuItem("Open");
                    ImGui::MenuItem("Save");
                    if(ImGui::MenuItem("Settings")) {
                        settings_clicked = true;
                    }
                    if(ImGui::MenuItem("Exit")) {
                        done = true;
                    }
                    ImGui::EndMenu();
                }

                if(ImGui::BeginMenu("View")) {
                    ImGui::MenuItem((const char*) Windows::BLE_Adapter::name.data(), nullptr, &menu_bar_enables.ble_adapter);

                    ImGui::MenuItem((const char*) Windows::Calibrate::name_base.data(), nullptr, &menu_bar_enables.calibrate);
                    ImGui::MenuItem((const char*) Windows::Measure::name_base.data(), nullptr, &menu_bar_enables.measure);
                    ImGui::MenuItem((const char*) Windows::Auto::name_base.data(), nullptr, &menu_bar_enables.auto_window);

                    ImGui::MenuItem((const char*) Windows::Plots::Measurement::name_base.data(), nullptr, &menu_bar_enables.measurement_plots);
                    ImGui::MenuItem((const char*) Windows::Plots::Calibration::name_base.data(), nullptr, &menu_bar_enables.calibration_plots);
                    ImGui::MenuItem((const char*) Windows::Plots::Auto::name_base.data(), nullptr, &menu_bar_enables.auto_plots);

                    ImGui::MenuItem((const char*) Windows::Console::name.data(), nullptr, &menu_bar_enables.console);
                    ImGui::MenuItem((const char*) Windows::Debug::name_base.data(), nullptr, &menu_bar_enables.debug);
                    ImGui::MenuItem("Demo", nullptr, &menu_bar_enables.demo);
                    ImGui::EndMenu();
                }

                if(ImGui::BeginMenu("Help")) {
                    if(ImGui::MenuItem("Legal")) {
                        legal_clicked = true;
                    }
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
                popup_enables.settings = true;
            }

            if(ImGui::BeginPopupModal("Settings", &popup_enables.settings, ImGuiWindowFlags_AlwaysAutoResize)) {
                draw_settings();
                ImGui::EndPopup();
            }

            if(about_clicked) {
                ImGui::OpenPopup("About");
                // Always center this window when appearing
                const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                about_clicked = false;
                popup_enables.about = true;
            }

            if(ImGui::BeginPopupModal("About", &popup_enables.about, ImGuiWindowFlags_AlwaysAutoResize)) {
                draw_about_page();
                ImGui::EndPopup();
            }

            if(legal_clicked) {
                ImGui::OpenPopup("Legal");
                // Always center this window when appearing
                const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                legal_clicked = false;
                popup_enables.legal = true;
            }

            if(ImGui::BeginPopupModal("Legal", &popup_enables.legal, ImGuiWindowFlags_AlwaysAutoResize)) {
                draw_legal();
                ImGui::EndPopup();
            }

            ImGui::End();
        }

        return dockspace_id;
    }

    void Top::draw_about_page() {
        ImGui::Text("Version: " "0.1.6");
        ImGui::Text("Built on: " __DATE__ " " __TIME__);

        static const char github_link[] { "[Contribute](https://www.github.com/spikeyamk/ESP32_AD5933)" };
        ImGui::RenderMarkdown(github_link, sizeof(github_link) / sizeof(github_link[0]));
    }

    void Top::draw_license(const char* name, const char* license) const {
        if(ImGui::TreeNode(name)) {
            ImGui::Text("%s", license);
            ImGui::TreePop();
        }
    }

    void Top::draw_legal() const {
        draw_license(
            "SimpleBLE"
            "          "
            "          "
            "          "
            "          "
            "          "
            "          "
            "          "
            "          "
            "          "
            "          "
            "          "
            "          "
            "          "
            "          "
            "          "
            "          "
            "          "
            , Legal::simpleble
        );
        draw_license("Dear ImGui", Legal::dear_imgui);
        draw_license("implot", Legal::implot);
        draw_license("json", Legal::json);
        draw_license("nativefiledialog-extended", Legal::nativefiledialog_extended);
        draw_license("boost", Legal::boost);
        draw_license("sml", Legal::sml);
        draw_license("SDL3", Legal::sdl3);
        draw_license("semver", Legal::semver);
        draw_license("utfconv", Legal::utfconv);
        draw_license("fmt", Legal::fmt);
        draw_license("Ubuntu-Sans-fonts", Legal::ubuntu_sans_fonts);
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

        if(ImGui::Combo("Scale", &scale_combo, ns::Settings::Scales::identifiers.data())) {
            update_scale();
        }

        ImGui::Separator();
        ImGui::Checkbox("Local Time", &ImPlot::GetStyle().UseLocalTime);
        ImGui::SameLine();
        ImGui::Checkbox("ISO 8601", &ImPlot::GetStyle().UseISO8601);
        ImGui::SameLine();
        ImGui::Checkbox("24 Hour Clock", &ImPlot::GetStyle().Use24HourClock);
        ImGui::Separator();

        if(ImGui::Button("OK", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
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
                Trielo::trielo_lambda<SDL_PushEvent>(Trielo::OkErrCode(0), Boilerplate::sdl_error_lambda, &event);
                break;
            case 1:
                Trielo::trielo<ImGui::StyleColorsDark>(nullptr);
                break;
            case 2:
                Trielo::trielo<ImGui::StyleColorsLight>(nullptr);
                break;
            case 3:
                Trielo::trielo<ImGui::StyleColorsClassic>(nullptr);
                break;
        }
    }

    void Top::update_scale() {
        Boilerplate::respect_system_scale = (scale_combo == 0 ? true : false);
        SDL_Event event;
        SDL_zero(event);
        if(scale_combo != 0) {
            event.type = SDL_EVENT_USER;
            event.user.code = *reinterpret_cast<const Uint32*>(ns::Settings::Scales::values + static_cast<size_t>(scale_combo - 1));
            Trielo::trielo_lambda<SDL_PushEvent>(Trielo::OkErrCode(0), Boilerplate::sdl_error_lambda, &event);
        } else {
            event.type = SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED;
            Trielo::trielo_lambda<SDL_PushEvent>(Trielo::OkErrCode(0), Boilerplate::sdl_error_lambda, &event);
        }
    }
}