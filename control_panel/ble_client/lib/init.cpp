#include "ble_client/ostream_overloads.hpp"

#include "ble_client/init.hpp"

namespace BLE_Client {
    std::optional<SimpleBLE::Adapter> find_default_active_adapter(std::shared_ptr<BLE_Client::SHM::SHM> shm) {
        if(SimpleBLE::Adapter::bluetooth_enabled() == false) {
            shm->console.log("WARNING: BLE_Client::find_default_active_adapter:: Bleutooth disabled.\n");
            return std::nullopt;
        }

        auto adapters = SimpleBLE::Adapter::get_adapters();
        if(adapters.empty()) {
            shm->console.log("WARNING: BLE_Client::find_default_active_adapter:: Could not find any adapter: SimpleBLE::Adapter::get_adapters: returns empty.\n");
            return std::nullopt;
        }

        for(SimpleBLE::Adapter& e: adapters) {
            if(e.bluetooth_enabled()) {
                return e;
            }
        }
        shm->console.log("WARNING: BLE_Client::find_default_active_adapter:: Could not find an enabled Bluetooth adpater.\n");
        return std::nullopt;
    }
}
