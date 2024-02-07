#pragma once

#include <vector>
#include <memory>

#include <boost/interprocess/managed_shared_memory.hpp>

#include "ble_client/state_machines/adapter/states.hpp"
#include "ble_client/state_machines/connector/events.hpp"
#include "ble_client/shm/child/specialized.hpp"

namespace BLE_Client {
    namespace SHM {
        class ChildSHM {
        private:
            boost::interprocess::managed_shared_memory segment;
        public: 
            CMD_ChannelRX cmd;
            ConsoleChannelTX console;
            DiscoveryDevices* discovery_devices;
            struct Device {
                std::shared_ptr<NotifyChannelTX> measurement { nullptr };
                std::shared_ptr<NotifyChannelTX> information { nullptr };
            };
            std::vector<Device> active_devices;
            BLE_Client::StateMachines::Adapter::States::T_Variant* active_state;
            ChildSHM();
            void init_device(const BLE_Client::StateMachines::Connector::Events::connect& connect_event);
        };
    }
}

