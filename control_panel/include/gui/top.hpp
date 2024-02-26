#pragma once

#include <string_view>

#include "imgui.h"

#include "json/settings.hpp"

namespace GUI {
    class Top {
    private:
        int theme_combo { Boilerplate::respect_system_theme ? 0 : 1 };
        int scale_combo {  Boilerplate::respect_system_scale ? 0 : 3 };

        struct Scales {
            static constexpr float values[] {
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

            static constexpr std::string_view identifiers {
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
        };
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
            bool file_manager { true };
            bool measurement_plots { true };
            bool calibration_plots { true };
            bool auto_plots { true };
            bool console { false };
            bool demo { false };
        };
        MenuBarEnables menu_bar_enables {};
        Top();
        ImGuiID draw(bool& done);
    private:
        void draw_about_page();
        void draw_settings();
        void draw_legal() const;
        void draw_license(const char* name, const char* license) const;
    private:
        void save_settings();
        void update_theme();
        void update_scale();
    };
}