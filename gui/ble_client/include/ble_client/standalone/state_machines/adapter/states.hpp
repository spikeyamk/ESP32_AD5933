#pragma once

#include <variant>
#include <string>
#include <map>

namespace BLE_Client {
    namespace StateMachines {
        namespace Adapter {
            namespace States {
                struct off{};
                struct on{};
                struct discovering{};
                using T_Variant = std::variant<off, on, discovering>;

                #ifdef _MSC_VER
                    const std::string prefix = "_1_9::aux::get_type_name<struct BLE_Client::StateMachines::Adapter::States::";
                #else
                    const std::string prefix = "BLE_Client::StateMachines::Adapter::States::";
                #endif
                extern std::map<std::string, T_Variant> stupid_sml;
            }
        }
    }
}
