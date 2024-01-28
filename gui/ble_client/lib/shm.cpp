#include "ble_client/standalone/shm.hpp"

namespace BLE_Client {
    namespace SHM {
        CMD_ChannelTX::CMD_ChannelTX(const char* name, boost::interprocess::managed_shared_memory& segment) :
            RelationBase{ name, segment },
            DirectionBase{ name, get_init_deque_ptr() }
        {}

        void CMD_ChannelTX::send_killer(const BLE_Client::StateMachines::Killer::Events::T_Variant& event) {
            send( BLE_Client::StateMachines::T_EventsVariant{ event } );
        }

        void CMD_ChannelTX::send_adapter(const BLE_Client::StateMachines::Adapter::Events::T_Variant& event) {
            send( BLE_Client::StateMachines::T_EventsVariant{ event } );
        }
        
        CMD_ChannelRX::CMD_ChannelRX(const char* name, boost::interprocess::managed_shared_memory& segment) :
            RelationBase{ name, segment },
            DirectionBase{ name, get_attach_deque_ptr() }
        {}

        NotifyChannelRX::NotifyChannelRX(const char* name, boost::interprocess::managed_shared_memory& segment) :
            RelationBase{ name, segment },
            DirectionBase{ name, get_attach_deque_ptr() }
        {}

        NotifyChannelTX::NotifyChannelTX(const char* name, boost::interprocess::managed_shared_memory& segment) :
            RelationBase{ name, segment },
            DirectionBase{ name, get_init_deque_ptr() }
        {}
    }

    namespace SHM {
        ParentSHM::ParentSHM() :
            segment{boost::interprocess::create_only, Names::shm, size},
            cmd{ Names::cmd_postfix, segment },
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

        void ParentSHM::send_packet(const size_t index, const Magic::Packets::Packet_T &packet) {
            cmd.send(BLE_Client::StateMachines::Connection::Events::write{ index, packet });
        }
    }

    namespace SHM {
        ChildSHM::ChildSHM() :
            segment{ boost::interprocess::managed_shared_memory(boost::interprocess::open_only, Names::shm) },
            cmd{ Names::cmd_postfix, segment },
            discovery_devices{ segment.find<DiscoveryDevices>(Names::discovery_devices).first },
            active_state{ [&]() {
                auto* tmp = segment.find<BLE_Client::StateMachines::Adapter::States::T_Variant>(Names::adapter_active_state).first;
                if(tmp == nullptr) {
                    throw std::invalid_argument(std::string("BLE_Clienth::SHM::ChildSHM::ChildSHM(): failed to attach: ").append(Names::adapter_active_state));
                }
                return tmp;
            }() }
        {}

        void ChildSHM::init_notify_channel(const BLE_Client::StateMachines::Connector::Events::connect& connect_event) {
            notify_channels.push_back(std::make_shared<NotifyChannelTX>(connect_event.get_address_dots_instead_of_colons().c_str(), segment));
        }
    }
}