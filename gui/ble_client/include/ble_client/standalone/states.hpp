#pragma once

#include <variant>
#include <map>
#include <string>
#include <utility>

namespace BLE_Client {
    namespace Discovery {
        namespace States {
            struct off{};
            struct on{};
            struct using_adapter{};
            struct discovering{};
            struct not_discovered{};
            struct discovered{};
            struct discovery_finished{};
            struct alive{};
            struct dead{};
            struct connecting{};
            struct connected{};
            struct disconnecting{};
            struct disconnected{};
            struct error{};
            struct using_esp32_ad5933{};
            using T_State = std::variant<
                off, on, using_adapter,
                discovering, not_discovered, discovered,
                discovery_finished, alive, dead,
                connecting, connected, disconnecting,
                disconnected, error, using_esp32_ad5933
            >;
            #ifdef _MSC_VER
                const std::string prefix = "_1_9::aux::get_type_name<struct BLE_Client::Discovery::States::";
            #else
                const std::string prefix = "BLE_Client::Discovery::States::";
            #endif
            extern std::map<std::string, T_State> stupid_sml;
        }
    }
}