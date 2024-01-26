#pragma once

#include <variant>

namespace BLE_Client {
    namespace StateMachines {
        namespace Killer {
            namespace Events {
                struct kill{};
                using T_Variant = std::variant<
                    kill
                >;
            }
        }
    }
}