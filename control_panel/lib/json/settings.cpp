#include <cstdlib>

#include "json/settings.hpp"

namespace ns {
    void to_json(json& j, const Settings& p) {
        j = json {
            { Settings::Names::theme_combo, p.theme_combo },
            { Settings::Names::scale_combo, p.scale_combo },
            { Settings::Names::local_time, p.local_time },
            { Settings::Names::iso_8601, p.iso_8601 },
            { Settings::Names::twenty_four_hour_clock, p.twenty_four_hour_clock },
        };
    }

    void from_json(const json& j, Settings& p) {
        j.at(Settings::Names::theme_combo).get_to(p.theme_combo);
        j.at(Settings::Names::scale_combo).get_to(p.scale_combo);
        j.at(Settings::Names::local_time).get_to(p.local_time);
        j.at(Settings::Names::iso_8601).get_to(p.iso_8601);
        j.at(Settings::Names::twenty_four_hour_clock).get_to(p.twenty_four_hour_clock);
    }
}

namespace ns {
    void to_json(json& j, const SettingsFile& p) {
        j = json {
            { p.name, p.settings },
        };
    }

    void from_json(const json& j, SettingsFile& p) {
        j.at(p.name).get_to(p.settings);
    }

    const std::filesystem::path settings_file_path {
        []() {
            const std::filesystem::path app_data_dir_path {
                std::filesystem::path(
                    std::getenv(
                        #ifdef _MSC_VER
                        "APPDATA"
                        #else
                        "XDG_CONFIG_HOME"
                        #endif
                    )
                ).append("esp32_ad5933_control_panel")
            };
            std::filesystem::create_directory(app_data_dir_path);
            return std::filesystem::path(app_data_dir_path).append(std::string("settings.json"));
        }()
    };
}