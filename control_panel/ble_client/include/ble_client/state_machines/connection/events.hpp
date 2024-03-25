#pragma once

#include <variant>
#include <cstddef>
#include <array>
#include <cstdint>

#include "magic/common.hpp"
#include "magic/commands/commands.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Connection {
            namespace Events {
                struct disconnect{
                    size_t index;
                };
                struct write_body_composition_feature{
                    size_t index;
                    Magic::Commands::Pack::apply_to<std::variant> event_variant;
                };
                using T_Variant = std::variant<disconnect, write_body_composition_feature>;
            }
        }
    }
}