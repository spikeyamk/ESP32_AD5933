#pragma once

#include "ble_client/shm/common/common.hpp"
#include "ble_client/shm/common/channel_types/base.hpp"
#include "ble_client/shm/common/channel_types/deque.hpp"
#include "ble_client/state_machines/events_variant.hpp"
#include "magic/events/results.hpp"

namespace BLE_Client {
    namespace SHM {
        class CMD_ChannelRX : public T_AttachChannel<Deque<BLE_Client::StateMachines::T_EventsVariant>>, public RX_DequeChannel<BLE_Client::StateMachines::T_EventsVariant> {
            using RelationBase = T_AttachChannel<Deque<BLE_Client::StateMachines::T_EventsVariant>>;
            using DirectionBase = RX_DequeChannel<BLE_Client::StateMachines::T_EventsVariant>;
        public:
            CMD_ChannelRX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
        };

        class NotifyChannelTX: public T_InitChannel<Deque<Magic::Events::Results::Variant>>, public TX_DequeChannel<Magic::Events::Results::Variant> {
            using RelationBase = T_InitChannel<Deque<Magic::Events::Results::Variant>>;
            using DirectionBase = TX_DequeChannel<Magic::Events::Results::Variant>;
        public:
            NotifyChannelTX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
        };

        class ConsoleChannelTX : public T_AttachChannel<String> {
        private:
            using Base = T_AttachChannel<String>;
        public:
            ConsoleChannelTX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
            void log(const std::string& message);
        };
    }
}

