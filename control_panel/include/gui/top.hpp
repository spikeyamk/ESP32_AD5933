#pragma once

#include <string_view>

#include <imgui.h>
#include <SDL3/SDL.h>

#include "gui/boilerplate.hpp"
#include "json/settings.hpp"
#include "legal/legal.hpp"

namespace GUI {
    class Top {
    private:
        int theme_combo { Boilerplate::respect_system_theme ? 0 : 1 };
        int scale_combo { Boilerplate::respect_system_scale ? 0 : 3 };

        bool settings_clicked { false };
        bool about_clicked { false };
        bool legal_clicked { false };
    public:
        struct MenuBarEnables {
            bool ble_adapter { true };
            bool calibrate { true };
            bool measure { true };
            bool debug { false };
            bool auto_window { true };
            bool measurement_plots { true };
            bool calibration_plots { true };
            bool auto_plots { true };
            bool imgui_demo { false };
            bool implot_demo { false };
            bool implot_dense_test { false };
        };
        MenuBarEnables menu_bar_enables {};

        Top(const ns::SettingsFile& settings_file);
        ImGuiID draw(bool& event_quit);
    private:
        struct PopupEnables {
            bool legal { true };
            bool about { true };
            bool settings { true };
        };
        PopupEnables popup_enables {};

        void draw_about_page();
        void draw_settings();
        void draw_legal() const;
        void draw_license(const Legal::License& license) const;
    private:
        void save_settings();
        void update_theme();
        void update_scale();
    };
}