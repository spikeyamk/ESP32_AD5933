#include "ble_client/state_machines/adapter/states.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Adapter {
            namespace States {
                std::map<std::string, T_Variant> stupid_sml {
                    { { prefix + "off" }, off{} },
                    { { prefix + "on" }, on{} },
                    { { prefix + "discovering" }, discovering{} },
                };
            }
        }
    }
}