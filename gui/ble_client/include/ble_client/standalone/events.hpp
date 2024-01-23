#pragma once

#include <array>
#include <cstdint>
#include <variant>

namespace BLE_Client {
    namespace Discovery {
        namespace Events {
            struct turn_on{};
            struct find_default_active_adapter{};
            struct start_discovery{};
            struct stop_discovery{};
            struct kill{};
            struct connect{
                size_t i;
            };
            struct is_connected{};
            struct disconnect{};
            struct is_esp32_ad5933{};
            struct stop_using_esp32_ad5933{};
            struct esp32_ad5933_write{
                std::array<uint8_t, 20> packet;
            };
            using T_Variant = std::variant<
                turn_on,
                find_default_active_adapter,
                start_discovery,
                stop_discovery,
                kill,
                connect,
                is_connected,
                disconnect,
                is_esp32_ad5933,
                stop_using_esp32_ad5933,
                esp32_ad5933_write
            >;
        }
    }
}
