#include "ble_client/shm/parent/specialized.hpp"

namespace BLE_Client {
    namespace SHM {
        CMD_ChannelTX::CMD_ChannelTX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
            RelationBase{ name, segment },
            DirectionBase{ name, get_init_ptr() }
        {}

        void CMD_ChannelTX::send_killer(const BLE_Client::StateMachines::Killer::Events::T_Variant& event) {
            send( BLE_Client::StateMachines::T_EventsVariant{ event } );
        }

        void CMD_ChannelTX::send_adapter(const BLE_Client::StateMachines::Adapter::Events::T_Variant& event) {
            send( BLE_Client::StateMachines::T_EventsVariant{ event } );
        }

        NotifyChannelRX::NotifyChannelRX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
            RelationBase{ name, segment },
            DirectionBase{ name, get_attach_ptr() }
        {}

        ConsoleChannelRX_Interface::ConsoleChannelRX_Interface(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
            Base{ name, segment }
        {}

        std::optional<std::string> ConsoleChannelRX_Interface::read_for(const boost::posix_time::milliseconds& timeout_ms) {
            boost::interprocess::scoped_lock lock(this->mutex);
            if(this->condition.timed_wait(lock, boost::get_system_time() + timeout_ms, [self = this]() {
                return self->data->size() != 0;
            }) == false) {
                return std::nullopt;
            }
            const std::string ret { *this->data };
            this->data->clear();
            return ret;
        }

        ConsoleChannelRX::ConsoleChannelRX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
            DirectionBase{ name, segment },
            RelationBase{ name, segment }
        {}
    }
}
