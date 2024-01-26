#pragma once

#include <mutex>
#include <memory>
#include <functional>

#include <boost/sml.hpp>
#include <simpleble/SimpleBLE.h>

#include "ble_client/standalone/state_machines/logger.hpp"
#include "ble_client/standalone/state_machines/connection/events.hpp"
#include "ble_client/standalone/shm.hpp"
#include "ble_client/standalone/esp32_ad5933.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Connection {
            namespace States {
                struct off{};
                struct connected{};
                struct disconnected{};
                struct connecting{};
                struct using_esp32_ad5933{};
            }

            namespace Actions {
                void connect(const BLE_Client::StateMachines::Connection::Events::connect& event, SimpleBLE::Adapter& adapter, SimpleBLE::Peripheral& peripheral, BLE_Client::SHM::SHM* shm);
                void disconnect(SimpleBLE::Peripheral& peripheral, ESP32_AD5933& esp32_ad5933);
                void write(const BLE_Client::StateMachines::Connection::Events::write& event, ESP32_AD5933& esp32_ad5933);
                void setup_subscriptions(ESP32_AD5933& esp32_ad5933);
            }
            
            namespace Guards {
                bool is_connected(SimpleBLE::Peripheral& peripheral);
                bool is_not_connected(SimpleBLE::Peripheral& peripheral);
                bool is_esp32_ad5933(SimpleBLE::Peripheral& peripheral, ESP32_AD5933& esp32_ad5933, BLE_Client::SHM::SHM* shm);
                bool is_not_esp32_ad5933(SimpleBLE::Peripheral& peripheral, ESP32_AD5933& esp32_ad5933, BLE_Client::SHM::SHM* shm);
            }

            struct Connection {
                auto operator()() const {
                    using namespace boost::sml;
                    using namespace std;
                    auto ret = make_transition_table(
                        *state<States::off> + event<Events::connect> / function{Actions::connect} = state<States::connecting>,
                        state<States::connecting> [function{Guards::is_connected}] = state<States::connected>,
                        state<States::connecting> [function{Guards::is_not_connected}] = state<States::disconnected>,
                        state<States::connected> [function{Guards::is_esp32_ad5933}] / function{Actions::setup_subscriptions} = state<States::using_esp32_ad5933>,
                        state<States::connected> [function{Guards::is_not_esp32_ad5933}] / function{Actions::disconnect} = state<States::disconnected>,
                        state<States::using_esp32_ad5933> + event<Events::write> / function{Actions::write} = state<States::using_esp32_ad5933>,
                        state<States::using_esp32_ad5933> + event<Events::disconnect> / function{Actions::disconnect} = state<States::disconnected>
                    );
                    return ret;
                }
            };
            using T_StateMachine = boost::sml::sm<Connection, boost::sml::logger<BLE_Client::StateMachines::Logger>, boost::sml::thread_safe<std::recursive_mutex>, boost::sml::testing>;
        }
    }
}
