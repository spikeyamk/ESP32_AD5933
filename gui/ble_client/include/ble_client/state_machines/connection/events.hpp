#pragma once

#include <variant>
#include <cstddef>
#include <array>
#include <cstdint>

#include "magic/events/common.hpp"
#include "magic/events/commands.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Connection {
            namespace Events {
                struct disconnect{
                    size_t index;
                };
                struct write_event{
                    size_t index;
                    Magic::Events::Commands::Variant event_variant;
                };
                using T_Variant = std::variant<disconnect, write_event>;
            }
        }
    }
}