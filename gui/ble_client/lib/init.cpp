#include <algorithm>

#include "ble_client/standalone/ostream_overloads.hpp"
#include <trielo/trielo.hpp>

#include "ble_client/standalone/init.hpp"

namespace BLE_Client {
    std::optional<SimpleBLE::Adapter> find_default_active_adapter() {
        if(Trielo::trielo<SimpleBLE::Adapter::bluetooth_enabled>(Trielo::OkErrCode(true)) == false) {
            std::cout << "ERROR: BLE: Bleutooth disabled\n";
            return std::nullopt;
        }

        auto adapters = Trielo::trielo<SimpleBLE::Adapter::get_adapters>();
        if(adapters.empty()) {
            std::cout << "ERROR: BLE: Could not find a Bluetooth adapter\n";
            return std::nullopt;
        }

        // Use the first adapter
        auto adapter = adapters[0];

        std::printf("BLE_Client::find_default_active_adapter:: looping over found adapters\n");

        const auto ret_it = std::find_if(adapters.begin(), adapters.end(), [index = 0](const auto& e) mutable {
            if(e.bluetooth_enabled() == false) {
                std::printf("\tBLE_Client::find_default_active_adapter::adapters[%d]: has disabled bluetooth\n", index);
                return false;
            } else {
                std::cout << "BLE_Client:: Found default active adapter\n ";
                return true;
            }
            index++;
        });

        if(ret_it == adapters.end()) {
            fmt::print(fmt::fg(fmt::color::yellow), "WARNING: BLE_Client::find_default_active_adapter:: Could not find default active adapter\n");
            return std::nullopt;
        }

        return *ret_it;
    }
}
