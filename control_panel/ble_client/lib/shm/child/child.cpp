#include "ble_client/shm/child/child.hpp"

namespace BLE_Client {
    namespace SHM {
        ChildSHM::ChildSHM() :
            segment{ boost::interprocess::managed_shared_memory(boost::interprocess::open_only, Names::shm) },
            cmd{ Names::cmd_postfix, segment },
            console{ Names::log_postfix, segment },
            discovery_devices{ segment.find<DiscoveryDevices>(Names::discovery_devices).first },
            active_state{ [&]() {
                auto* tmp = segment.find<BLE_Client::StateMachines::Adapter::States::T_Variant>(Names::adapter_active_state).first;
                if(tmp == nullptr) {
                    throw std::invalid_argument(std::string("BLE_Clienth::SHM::ChildSHM::ChildSHM(): failed to attach: ").append(Names::adapter_active_state));
                }
                return tmp;
            }() }
        {}

        void ChildSHM::init_device(const BLE_Client::StateMachines::Connector::Events::connect& connect_event) {
            const Device tmp_device {
                std::make_shared<NotifyChannelTX>(connect_event.get_measurement_name().c_str(), segment),
                std::make_shared<NotifyChannelTX>(connect_event.get_information_name().c_str(), segment)
            };
            active_devices.push_back(tmp_device);
        }
    }
}