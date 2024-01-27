#pragma once

#include <optional>
#include <memory>

#include <boost/process.hpp>

#include "ble_client/ble_client.hpp"
#include "ble_client/standalone/shm.hpp"

namespace GUI {
    void run(bool &done, boost::process::child& ble_client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
}