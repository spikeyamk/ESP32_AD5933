#pragma once

#include <stop_token>
#include <memory>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>

#include <simpleble/SimpleBLE.h>
#include <boost/sml.hpp>

#include "ble_client/state_machines/adapter/adapter.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Adapter {
            void checker(
                std::stop_source stop_source,
                BLE_Client::StateMachines::Adapter::T_StateMachine& adapter_sm,
                SimpleBLE::Adapter& adapter,
                std::shared_ptr<BLE_Client::SHM::ChildSHM> shm
            );
        }
    }
}