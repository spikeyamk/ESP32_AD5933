#include "ble_client/state_machines/killer/killer.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Killer {
            namespace Actions {
                void kill(std::stop_source stop_source) {
                    stop_source.request_stop();
                }
            }
        }
    }
}