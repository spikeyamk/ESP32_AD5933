#include "ble_client/shm/specialized.hpp"

namespace BLE_Client {
    namespace SHM {
        CMD_ChannelTX::CMD_ChannelTX() :
            DirectionBase{ }
        {}

        void CMD_ChannelTX::send_killer(const BLE_Client::StateMachines::Killer::Events::T_Variant& event) {
            send( BLE_Client::StateMachines::T_EventsVariant{ event } );
        }

        void CMD_ChannelTX::send_adapter(const BLE_Client::StateMachines::Adapter::Events::T_Variant& event) {
            send( BLE_Client::StateMachines::T_EventsVariant{ event } );
        }

        NotifyChannelRX::NotifyChannelRX() :
            DirectionBase{ }
        {}

        ConsoleChannelRX_Interface::ConsoleChannelRX_Interface() :
            Base{ }
        {}

        std::optional<std::string> ConsoleChannelRX_Interface::read_for(const std::chrono::milliseconds& timeout_ms) {
            std::unique_lock lock(this->mutex);
            if(this->condition.wait_for(lock, timeout_ms, [self = this]() {
                return self->data.size() != 0;
            }) == false) {
                return std::nullopt;
            }
            const std::string ret { this->data };
            this->data.clear();
            return ret;
        }

        ConsoleChannelRX::ConsoleChannelRX() :
            DirectionBase{ }
        {}
    }
}
