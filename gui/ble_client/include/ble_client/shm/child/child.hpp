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
            std::vector<std::shared_ptr<NotifyChannelTX>> notify_channels;
            BLE_Client::StateMachines::Adapter::States::T_Variant* active_state;
            ChildSHM();
            void init_notify_channel(const BLE_Client::StateMachines::Connector::Events::connect& connect_event);
        };
    }
}

