#pragma once

#include <vector>
#include <memory>

#include "ble_client/shm/specialized.hpp"
#include "ble_client/state_machines/adapter/states.hpp"

namespace BLE_Client {
    namespace SHM {
        class SHM {
        public:
            CMD_ChannelTX cmd;
            ConsoleChannelRX console;
            DiscoveryDevices discovery_devices;
            struct Device {
                std::shared_ptr<NotifyChannelRX> measurement { nullptr };
                std::shared_ptr<NotifyChannelRX> information { nullptr };
            };
            std::shared_ptr<std::vector<Device>> active_devices { std::make_shared<std::vector<Device>>() };
            BLE_Client::StateMachines::Adapter::States::T_Variant active_state;
            SHM() = default;
            void attach_device(const BLE_Client::StateMachines::Adapter::Events::connect& connect_event);
        };
    }
}
