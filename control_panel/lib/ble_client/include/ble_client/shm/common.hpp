#pragma once

#include <vector>
#include <string>

#include "ble_client/device.hpp"

namespace BLE_Client {
    namespace SHM {
        using DiscoveryDevices = std::vector<BLE_Client::Discovery::Device>;
        using String = std::string;
    }
}