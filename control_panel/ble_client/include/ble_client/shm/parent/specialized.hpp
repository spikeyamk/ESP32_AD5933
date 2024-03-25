#pragma once

#include <variant>

#include "ble_client/shm/common/common.hpp"
#include "ble_client/shm/common/channel_types/base.hpp"
#include "ble_client/shm/common/channel_types/deque.hpp"
#include "ble_client/state_machines/events_variant.hpp"
#include "magic/results/results.hpp"

namespace BLE_Client {
    namespace SHM {
        class CMD_ChannelTX : public T_ScopedInitChannel<Deque<BLE_Client::StateMachines::T_EventsVariant>>, public TX_DequeChannel<BLE_Client::StateMachines::T_EventsVariant> {
        public:
            using RelationBase = T_ScopedInitChannel<Deque<BLE_Client::StateMachines::T_EventsVariant>>;
            using DirectionBase = TX_DequeChannel<BLE_Client::StateMachines::T_EventsVariant>;
            CMD_ChannelTX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
            void send_killer(const BLE_Client::StateMachines::Killer::Events::T_Variant& event);
            void send_adapter(const BLE_Client::StateMachines::Adapter::Events::T_Variant& event);
        };

        class NotifyChannelRX : public T_ScopedAttachChannel<Deque<Magic::Results::Pack::apply_to<std::variant>>>, public RX_DequeChannel<Magic::Results::Pack::apply_to<std::variant>> {
            using RelationBase = T_ScopedAttachChannel<Deque<Magic::Results::Pack::apply_to<std::variant>>>;
            using DirectionBase = RX_DequeChannel<Magic::Results::Pack::apply_to<std::variant>>;
        public:
            NotifyChannelRX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
        };

        class ConsoleChannelRX_Interface : public T_InitChannel<String> {
            using Base = T_InitChannel<String>;
        public:
            ConsoleChannelRX_Interface(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
            std::optional<std::string> read_for(const boost::posix_time::milliseconds& timeout_ms);
        };

        class ConsoleChannelRX : public ConsoleChannelRX_Interface, public T_ScopedChannel<String> {
            using DirectionBase = ConsoleChannelRX_Interface;
            using RelationBase = T_ScopedChannel<String>;
        public:
            ConsoleChannelRX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
        };
    }
}