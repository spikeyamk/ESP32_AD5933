#pragma once

#include <variant>

namespace BLE_Client {
    namespace StateMachines {
        namespace Adapter {
            namespace Events {
                struct turn_on{};
                struct start_discovery{};
                struct stop_discovery{};
                using T_Variant = std::variant<
                    turn_on,
                    start_discovery,
                    stop_discovery
                >;
            }
        }
    }
}