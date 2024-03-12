#include <cstdlib>

#include "ble_client/shm/common/names.hpp"

namespace BLE_Client {
    namespace SHM {
        namespace Names {
            const std::filesystem::path json_path {
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
                    return std::filesystem::path(app_data_dir_path).append(std::string(shm).append(json_postfix));
                }()
            };
        }
    }
}