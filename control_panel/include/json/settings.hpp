#pragma once

#include <cstdint>
#include <string_view>
#include <filesystem>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace ns {
    struct Settings {
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

        struct Names {
            static constexpr std::string_view theme_combo { "theme_combo" };
            static constexpr std::string_view scale_combo { "scale_combo" };
            static constexpr std::string_view local_time { "local_time" };
            static constexpr std::string_view iso_8601 { "iso_8601" };
            static constexpr std::string_view twenty_four_hour_clock { "twenty_four_hour_clock" };
        };
        int theme_combo { 0 };
        int scale_combo { 0 };
        bool local_time;
        bool iso_8601;
        bool twenty_four_hour_clock;
    };

    void to_json(json& j, const Settings& p);
    void from_json(const json& j, Settings& p);
}

namespace ns {
    struct SettingsFile {
        static constexpr std::string_view name { "settings" };
        Settings settings;
    };

    void to_json(json& j, const SettingsFile& p);
    void from_json(const json& j, SettingsFile& p);

    extern const std::filesystem::path settings_file_path;
}