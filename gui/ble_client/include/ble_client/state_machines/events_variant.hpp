#pragma once

#include <variant>

#include "ble_client/state_machines/adapter/events.hpp"
#include "ble_client/state_machines/connector/events.hpp"
#include "ble_client/state_machines/connection/events.hpp"
#include "ble_client/state_machines/killer/events.hpp"

namespace BLE_Client {
    namespace StateMachines {
        using T_EventsVariant = std::variant<
            BLE_Client::StateMachines::Adapter::Events::T_Variant,
            BLE_Client::StateMachines::Connector::Events::connect,
            BLE_Client::StateMachines::Connection::Events::T_Variant,
            BLE_Client::StateMachines::Killer::Events::T_Variant
        >;
    }
}