#pragma once

#include <stop_token>
#include <memory>
#include <vector>

#include <simpleble/SimpleBLE.h>

#include "ble_client/state_machines/adapter/adapter.hpp"
#include "ble_client/state_machines/killer/killer.hpp"
#include "ble_client/state_machines/connector/connector.hpp"
#include "ble_client/state_machines/connection/connection.hpp"

namespace BLE_Client {
    void cmd_listener(
        std::stop_source stop_source,
        std::shared_ptr<BLE_Client::SHM::ChildSHM> shm,
        BLE_Client::StateMachines::Killer::T_StateMachine& killer,
        BLE_Client::StateMachines::Adapter::T_StateMachine& adapter_sm,
        std::vector<decltype(BLE_Client::StateMachines::Connection::Dummy<int>::sm)*>& connections,
        SimpleBLE::Adapter& simpleble_adapter,
        BLE_Client::StateMachines::Connector::T_StateMachine& connector
    );
}