#pragma once

#include <cstddef>
#include <vector>
#include <memory>

#include <boost/interprocess/managed_shared_memory.hpp>

#include "ble_client/shm/parent/specialized.hpp"
#include "ble_client/state_machines/adapter/states.hpp"
#include "ble_client/state_machines/connector/events.hpp"

namespace BLE_Client {
    namespace SHM {
        class ParentSHM {
        private:
            static constexpr size_t size = 2 << 20;
            boost::interprocess::managed_shared_memory segment;
        public:
            CMD_ChannelTX cmd;
            ConsoleChannelRX console;
            DiscoveryDevices* discovery_devices;
            std::vector<std::shared_ptr<NotifyChannelRX>> notify_channels;
            BLE_Client::StateMachines::Adapter::States::T_Variant* active_state;
            ParentSHM();
            ~ParentSHM();
            void attach_notify_channel(const BLE_Client::StateMachines::Connector::Events::connect& connect_event);
        };
    }
}
