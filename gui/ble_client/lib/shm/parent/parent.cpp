#include <string>
#include <stdexcept>

#include "ble_client/shm/parent/parent.hpp"

namespace BLE_Client {
    namespace SHM {
        ParentSHM::ParentSHM() :
            segment{boost::interprocess::create_only, Names::shm, size},
            cmd{ Names::cmd_postfix, segment },
            console{ Names::log_postfix, segment },
            discovery_devices{ segment.construct<DiscoveryDevices>(Names::discovery_devices)(segment.get_segment_manager()) },
            active_state{ [&]() {
                auto* tmp { segment.construct<BLE_Client::StateMachines::Adapter::States::T_Variant>
                    (Names::adapter_active_state)
                    (BLE_Client::StateMachines::Adapter::States::off{})
                };

                if(tmp == nullptr) {
                    throw std::invalid_argument(std::string("BLE_Client::SHM::ParentSHM::ParentSHM(): ").append(Names::adapter_active_state).append(" was previously used"));
                }

                return tmp;
            }() }
        {}

        ParentSHM::~ParentSHM() {
            segment.destroy<DiscoveryDevices>(Names::discovery_devices);
            segment.destroy<BLE_Client::StateMachines::Adapter::States::T_Variant>(Names::adapter_active_state);
            boost::interprocess::shared_memory_object::remove(Names::shm);
        }

        void ParentSHM::attach_notify_channel(const BLE_Client::StateMachines::Connector::Events::connect& connect_event) {
            notify_channels.push_back(std::make_shared<NotifyChannelRX>(connect_event.get_address_dots_instead_of_colons().c_str(), segment));
        }
    }
}