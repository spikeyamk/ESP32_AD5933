#include "ble_client/cmd_listener.hpp"

namespace BLE_Client {
    void cmd_listener(
        std::stop_source stop_source,
        std::shared_ptr<BLE_Client::SHM::SHM> shm,
        BLE_Client::StateMachines::Killer::T_StateMachine& killer,
        BLE_Client::StateMachines::Adapter::T_StateMachine& adapter_sm,
        std::vector<BLE_Client::StateMachines::Connection::Dummy*>& connections,
        SimpleBLE::Adapter& simpleble_adapter
    ) {
        std::stop_token st { stop_source.get_token() };
        while(st.stop_requested() == false) {
            const auto read { shm->cmd.read() };

            std::visit([&killer, &adapter_sm, &connections, shm, &simpleble_adapter](auto&& event_variant) {
                using T_EventVariantDecay = std::decay_t<decltype(event_variant)>;
                if constexpr (std::is_same_v<T_EventVariantDecay, BLE_Client::StateMachines::Killer::Events::T_Variant>) {
                    std::visit([&killer](auto&& event) {
                        killer.process_event(event);
                    }, event_variant);
                } else if constexpr (std::is_same_v<T_EventVariantDecay, BLE_Client::StateMachines::Adapter::Events::T_Variant>) {
                    std::visit([&adapter_sm](auto&& event) {
                        adapter_sm.process_event(event);
                    }, event_variant);
                } else if constexpr (std::is_same_v<T_EventVariantDecay, BLE_Client::StateMachines::Connection::Events::T_Variant>) {
                    std::visit([&connections](auto&& event) {
                        using T_EventDecay = std::decay_t<decltype(event)>;
                        connections[event.index]->process_event(event);
                        if constexpr (std::is_same_v<T_EventDecay, BLE_Client::StateMachines::Connection::Events::disconnect>) {
                            if(event.index < connections.size()) {
                                delete connections[event.index];
                                connections.erase(connections.begin() + event.index);
                            }
                        }
                    }, event_variant);
                }
            }, read);
       }
    }
}