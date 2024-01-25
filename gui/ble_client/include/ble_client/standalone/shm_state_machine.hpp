#pragma once

#include "ble_client/standalone/state_machine.hpp"

namespace BLE_Client {
    namespace SHM {
        const char smko_name[] = "BLE_Client.SHM.smko";
        BLE_Client::Discovery::T_StateMachineExpanded* smko;
    }
}