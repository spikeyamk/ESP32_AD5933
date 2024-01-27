#pragma once

#include <variant>
#include <cstddef>
#include <array>
#include <cstdint>

namespace BLE_Client {
    namespace StateMachines {
        namespace Connection {
            namespace Events {
                struct disconnect{
                    size_t index;
                };
                struct write{
                    size_t index;
                    std::array<uint8_t, 20> packet;
                };
                using T_Variant = std::variant<disconnect, write>;
            }
        }
    }
}