#include "ble_client/standalone/states.hpp"

namespace BLE_Client {
    namespace Discovery {
        namespace States {
            std::map<std::string, T_State> stupid_sml {
                { { prefix + "off" }, off{} },
                { { prefix + "on" }, on{} },
                { { prefix + "using_adapter" }, using_adapter{} },
                { { prefix + "discovering" }, discovering{} },
                { { prefix + "not_discovered" }, not_discovered{} },
                { { prefix + "discovered" }, discovered{} },
                { { prefix + "discovery_finished" }, discovery_finished{} },
                { { prefix + "alive" }, alive{} },
                { { prefix + "dead" }, dead{} },
                { { prefix + "connecting" }, connecting{} },
                { { prefix + "connected" }, connected{} },
                { { prefix + "disconnecting" }, disconnecting{} },
                { { prefix + "disconnected" }, disconnected{} },
                { { prefix + "error" }, error{} },
                { { prefix + "using_esp32_ad5933" }, using_esp32_ad5933{} },
            };
        }
    }
}