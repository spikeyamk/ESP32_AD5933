#include <thread>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <string_view>
#include <variant>
#include <type_traits>
#include <vector>
#include <future>
#include <csignal>

#include <boost/process.hpp>
#include <boost/thread/thread_time.hpp>
#include <trielo/trielo.hpp>

#include "magic/packets/outcoming.hpp"
#include "magic/events/commands.hpp"
#include "ble_client/shm.hpp"
#include "ble_client/worker.hpp"
#include "ble_client/cmd_listener.hpp"
#include "ble_client/state_machines/adapter/adapter.hpp"
#include "ble_client/state_machines/adapter/checker.hpp"
#include "ble_client/state_machines/killer/killer.hpp"
#include "ble_client/state_machines/connector/connector.hpp"

#include "ble_client/test.hpp"

namespace BLE_Client {
    template<typename T_State>
    bool state_tester(std::shared_ptr<BLE_Client::SHM::ParentSHM> parent_shm) {
        bool success = false;
        std::visit([&](auto&& active_state) {
            using T_Decay = std::decay_t<decltype(active_state)>;
            if constexpr(std::is_same_v<T_Decay, T_State>) {
                success = true;
            }
        }, *parent_shm->active_state);
        return success;
    }

    int basic_test() {
        std::shared_ptr<BLE_Client::SHM::ParentSHM> parent_shm = nullptr;
        try {
            parent_shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
        } catch(...) {
            return -1;
        }

        std::jthread child_thread(BLE_Client::worker);
        child_thread.detach();

        parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::turn_on{});
        std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
        if(state_tester<BLE_Client::StateMachines::Adapter::States::on>(parent_shm) == false) {
            return -2;
        }

        parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::start_discovery{});
        std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
        if(state_tester<BLE_Client::StateMachines::Adapter::States::discovering>(parent_shm) == false) {
            return -3;
        }

        parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::stop_discovery{});
        std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
        if(state_tester<BLE_Client::StateMachines::Adapter::States::on>(parent_shm) == false) {
            return -4;
        }

        parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::start_discovery{});
        std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
        if(state_tester<BLE_Client::StateMachines::Adapter::States::discovering>(parent_shm) == false) {
            return -5;
        }

        parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::stop_discovery{});
        std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
        if(state_tester<BLE_Client::StateMachines::Adapter::States::on>(parent_shm) == false) {
            return -6;
        }

        if(parent_shm->discovery_devices->empty()) {
            return -7;
        }

        for(int i = 1; i <= 2; i++) {
            static constexpr std::string_view nimble_address { "40:4C:CA:43:11:B2" };
            if(std::find_if(parent_shm->discovery_devices->begin(), parent_shm->discovery_devices->end(), [&](const auto& e) {
                return nimble_address == e.get_address();
            }) == parent_shm->discovery_devices->end()) {
                return -1 - (10 * i);
            }
            
            auto connect_event { BLE_Client::StateMachines::Connector::Events::connect{ nimble_address } };
            parent_shm->cmd.send(connect_event);
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            try {
                parent_shm->attach_notify_channel(connect_event);
            } catch(...) {
                return -2 - (10 * i);
            }

            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_event{ 0, Magic::Events::Commands::Debug::Start{} });
            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_event{ 0, Magic::Events::Commands::Debug::Dump{} });
            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_event{ 0, Magic::Events::Commands::Debug::End{} });

            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            const auto dump_all_registers { parent_shm->notify_channels[0]->read_for(boost::posix_time::milliseconds(5'000)) };
            if(dump_all_registers.has_value() == false) {
                return -3 - (10 * i);
            }

            bool is_dump_all_registers = false;
            std::visit([&is_dump_all_registers](auto&& event) {
                using T_Decay = std::decay_t<decltype(event)>;
                if constexpr (std::is_same_v<T_Decay, Magic::Events::Results::Debug::Dump>) {
                    std::printf("BLE_Client::basic_test: dump_all_registers:");
                    std::for_each(event.registers_data.begin(), event.registers_data.end(), [index = 0](const auto e) mutable {
                        if(index % 8 == 0) {
                            std::printf("\n    ");
                        }
                        std::printf("0x%02X, ", e);
                        index++;
                    });
                    std::printf("\n");
                    is_dump_all_registers = true;
                }
            }, dump_all_registers.value());

            if(is_dump_all_registers == false) {
                return -4 - (10 * i);
            }

            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::disconnect{ 0 });
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            try {
                parent_shm->notify_channels.erase(parent_shm->notify_channels.begin());
            } catch(...) {
                return -5 - (10 * i);
            }
        }
        
        parent_shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
        std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
        return 0;
    }
}
