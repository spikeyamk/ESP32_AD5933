#pragma once

#include <iostream>
#include <optional>
#include <chrono>

#include "ble_client/shm/common/common.hpp"
#include "ble_client/shm/common/channel_types/base.hpp"
#include "ble_client/shm/common/channel_types/deque.hpp"
#include "ble_client/state_machines/events_variant.hpp"
#include "magic/results/results.hpp"

namespace BLE_Client {
    namespace SHM {
        class CMD_ChannelTX : public DequeChannel<BLE_Client::StateMachines::T_EventsVariant> {
        public:
            using DirectionBase = DequeChannel<BLE_Client::StateMachines::T_EventsVariant>;
            CMD_ChannelTX();
            void send_killer(const BLE_Client::StateMachines::Killer::Events::T_Variant& event);
            void send_adapter(const BLE_Client::StateMachines::Adapter::Events::T_Variant& event);
        };

        class NotifyChannelRX : public DequeChannel<Magic::Results::Pack::apply_to<std::variant>> {
            using DirectionBase = DequeChannel<Magic::Results::Pack::apply_to<std::variant>>;
        public:
            NotifyChannelRX();
        };

        class ConsoleChannelRX_Interface : public T_Channel<String> {
            using Base = T_Channel<String>;
        public:
            ConsoleChannelRX_Interface();
            std::optional<std::string> read_for(const std::chrono::milliseconds& timeout_ms);
        };

        class ConsoleChannelRX : public ConsoleChannelRX_Interface {
            using DirectionBase = ConsoleChannelRX_Interface;
        public:
            ConsoleChannelRX();
            static inline void log(auto in) {
                std::cout << in;
            }
        };
    }
}