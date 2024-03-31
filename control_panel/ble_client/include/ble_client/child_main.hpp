#pragma once

#include <memory>

#include "ble_client/shm/parent/parent.hpp"

namespace BLE_Client {
    int child_main(std::shared_ptr<BLE_Client::SHM::Parent> child_shm);
}