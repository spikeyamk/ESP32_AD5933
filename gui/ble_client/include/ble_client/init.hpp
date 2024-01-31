#pragma once

#include <optional>
#include <memory>
#include <simpleble/SimpleBLE.h>
#include "ble_client/shm.hpp"

namespace BLE_Client {
    std::optional<SimpleBLE::Adapter> find_default_active_adapter(std::shared_ptr<BLE_Client::SHM::ChildSHM> shm);
}
