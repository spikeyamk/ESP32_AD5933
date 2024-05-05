#include <string>

#include "ble_client/shm/shm.hpp"

namespace BLE_Client {
    namespace SHM {
        void SHM::attach_device(const BLE_Client::StateMachines::Adapter::Events::connect& connect_event) {
            const Device tmp_device {
                std::make_shared<NotifyChannelRX>(),
                std::make_shared<NotifyChannelRX>()
            };
            active_devices->push_back(tmp_device);
        }
    }
}