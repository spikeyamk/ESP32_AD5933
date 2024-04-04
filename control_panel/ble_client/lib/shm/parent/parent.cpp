#include <string>

#include "ble_client/shm/parent/parent.hpp"

namespace BLE_Client {
    namespace SHM {
        void Parent::attach_device(const BLE_Client::StateMachines::Connector::Events::connect& connect_event) {
            const Device tmp_device {
                std::make_shared<NotifyChannelRX>(),
                std::make_shared<NotifyChannelRX>()
            };
            active_devices.push_back(tmp_device);
        }
    }
}