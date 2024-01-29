#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

#include "ble_client/standalone/ostream_overloads.hpp"

#include "ble_client/standalone/init.hpp"

namespace BLE_Client {
    std::optional<SimpleBLE::Adapter> find_default_active_adapter() {
        if(SimpleBLE::Adapter::bluetooth_enabled() == false) {
            std::printf("WARNING: BLE_Client::find_default_active_adapter:: Bleutooth disabled\n");
            return std::nullopt;
        }

        auto adapters = SimpleBLE::Adapter::get_adapters();
        if(adapters.empty()) {
            std::printf("WARNING: BLE_Client::find_default_active_adapter:: Could not find any adapter: SimpleBLE::Adapter::get_adapters: returns empty\n");
            return std::nullopt;
        }

        for(SimpleBLE::Adapter& e: adapters) {
            if(e.bluetooth_enabled()) {
                return e;
            }
        }
        std::printf("WARNING: BLE_Client::find_default_active_adapter:: Could not find an enabled Bluetooth adpater\n");
        return std::nullopt;
    }
}
