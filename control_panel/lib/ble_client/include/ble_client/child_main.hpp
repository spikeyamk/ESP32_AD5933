#pragma once

#include <memory>

#include "ble_client/shm/shm.hpp"

namespace BLE_Client {
    int child_main(std::shared_ptr<BLE_Client::SHM::SHM> shm);
}