#pragma once

#include <variant>

namespace BLE_Client {
    namespace StateMachines {
        namespace Adapter {
            namespace States {
                struct off{};
                struct on{};
                struct discovering{};
                using T_Variant = std::variant<off, on, discovering>;
            }
        }
    }
}
