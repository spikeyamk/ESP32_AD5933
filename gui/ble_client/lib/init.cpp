#include <algorithm>
#include <thread>
#include <chrono>

#include "ble_client/standalone/ostream_overloads.hpp"
#include <trielo/trielo.hpp>

#include "ble_client/standalone/init.hpp"

namespace BLE_Client {
    std::optional<SimpleBLE::Adapter> find_default_active_adapter() {
        if(Trielo::trielo<SimpleBLE::Adapter::bluetooth_enabled>(Trielo::OkErrCode(true)) == false) {
            fmt::print(fmt::fg(fmt::color::yellow), "WARNING: BLE_Client::find_default_active_adapter:: Bleutooth disabled\n");
            return std::nullopt;
        }

        auto adapters = Trielo::trielo<SimpleBLE::Adapter::get_adapters>();
        if(adapters.empty()) {
            fmt::print(fmt::fg(fmt::color::yellow), "WARNING: BLE_Client::find_default_active_adapter:: Could not find any adapter: SimpleBLE::Adapter::get_adapters: returns empty\n");
            return std::nullopt;
        }

        for(SimpleBLE::Adapter& e: adapters) {
            if(e.bluetooth_enabled()) {
                return e;
            }
        }
        fmt::print(fmt::fg(fmt::color::yellow), "WARNING: BLE_Client::find_default_active_adapter:: Could not find an enabled Bluetooth adpater\n");
        return std::nullopt;
    }
}
