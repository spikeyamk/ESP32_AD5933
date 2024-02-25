#pragma once

#include <cstdint>
#include <string_view>
#include <filesystem>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace ns {
    struct Settings {
        struct Names {
            static constexpr std::string_view theme_combo { "theme_combo" };
            static constexpr std::string_view scale_combo { "scale_combo" };
            static constexpr std::string_view local_time { "local_time" };
            static constexpr std::string_view iso_8601 { "iso_8601" };
            static constexpr std::string_view twenty_four_hour_clock { "twenty_four_hour_clock" };
        };
        int theme_combo;
        int scale_combo;
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