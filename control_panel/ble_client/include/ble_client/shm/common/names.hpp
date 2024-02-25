#pragma once

namespace BLE_Client {
    namespace SHM {
        namespace Names {
            constexpr char shm[] { "BLE_Client.shm" };
            constexpr char cmd_postfix[] { "cmd" };
            constexpr char log_postfix[] { "log" };
            constexpr char discovery_devices[] { "BLE_Client.shm.discovery_devices" };
            constexpr char adapter_active_state[] = "BLE_Client.SHM.adapter_active_state";
            constexpr char json_postfix[] { ".json" };
        }
    }
}