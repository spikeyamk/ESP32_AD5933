#pragma once

#include <mutex>
#include <memory>
#include <functional>

#include <boost/sml.hpp>
#include <simpleble/SimpleBLE.h>

#include "ble_client/state_machines/logger.hpp"
#include "ble_client/state_machines/connection/events.hpp"
#include "ble_client/shm/child/child.hpp"
#include "ble_client/esp32_ad5933.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Connection {
            namespace States {
                struct off{};
                struct connected{};
                struct disconnected{};
            }

            namespace Actions {
                void disconnect(std::shared_ptr<ESP32_AD5933> esp32_ad5933, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm);
            }
            
            namespace Guards {
                bool write_event_successful(const BLE_Client::StateMachines::Connection::Events::write_event& event, std::shared_ptr<ESP32_AD5933> esp32_ad5933, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm);
                bool write_event_failed(const BLE_Client::StateMachines::Connection::Events::write_event& event, std::shared_ptr<ESP32_AD5933> esp32_ad5933, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm);
            }

            struct Connection {
                auto operator()() const {
                    using namespace boost::sml;
                    using namespace std;
                    auto ret = make_transition_table(
                        *state<States::off> = state<States::connected>,
                        state<States::connected> + event<Events::disconnect>                                             / function{Actions::disconnect} = state<States::disconnected>,
                        state<States::connected> + event<Events::write_event> [function{Guards::write_event_successful}]                                 = state<States::connected>,
                        state<States::connected> + event<Events::write_event> [function{Guards::write_event_failed}]     / function{Actions::disconnect} = state<States::disconnected>
                    );
                    return ret;
                }
            };
            using T_StateMachine = boost::sml::sm<Connection, boost::sml::logger<BLE_Client::StateMachines::Logger>, boost::sml::thread_safe<std::recursive_mutex>, boost::sml::testing>;

            /* Super ugly hack this is stupid, but I can't for the love of god find the full type of the T_StateMachine *
             * The boost::sml library uses probably some SFINAE or some other complex bullshit I'm too stupid to understand */
            template<typename T>
            struct Dummy {
                std::shared_ptr<BLE_Client::ESP32_AD5933> esp32_ad5933;
                BLE_Client::StateMachines::Logger logger;
                T_StateMachine sm { esp32_ad5933, logger };
            };
        }
    }
}
