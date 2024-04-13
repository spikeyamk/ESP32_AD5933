#pragma once

#include <mutex>
#include <memory>
#include <functional>

#include <boost/sml.hpp>
#include <simpleble/SimpleBLE.h>

#include "ble_client/state_machines/logger.hpp"
#include "ble_client/state_machines/connection/events.hpp"
#include "ble_client/shm/shm.hpp"
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
                void disconnect(std::shared_ptr<ESP32_AD5933> esp32_ad5933, std::shared_ptr<BLE_Client::SHM::SHM> shm);
            }
            
            namespace Guards {
                bool write_body_composition_feature_successful(const BLE_Client::StateMachines::Connection::Events::write_body_composition_feature& event, std::shared_ptr<ESP32_AD5933> esp32_ad5933, std::shared_ptr<BLE_Client::SHM::SHM> shm);
                bool write_body_composition_feature_failed(const BLE_Client::StateMachines::Connection::Events::write_body_composition_feature& event, std::shared_ptr<ESP32_AD5933> esp32_ad5933, std::shared_ptr<BLE_Client::SHM::SHM> shm);
            }

            struct Connection {
                auto operator()() const {
                    using namespace boost::sml;
                    using namespace std;
                    auto ret = make_transition_table(
                        *state<States::off> = state<States::connected>,
                        state<States::connected> + event<Events::disconnect>                                             / function{Actions::disconnect} = state<States::disconnected>,
                        state<States::connected> + event<Events::write_body_composition_feature> [function{Guards::write_body_composition_feature_successful}]                                 = state<States::connected>,
                        state<States::connected> + event<Events::write_body_composition_feature> [function{Guards::write_body_composition_feature_failed}]     / function{Actions::disconnect} = state<States::disconnected>
                    );
                    return ret;
                }
            };
            using T_StateMachine = boost::sml::sm<Connection, boost::sml::logger<BLE_Client::StateMachines::Logger>, boost::sml::thread_safe<std::recursive_mutex>, boost::sml::testing>;

            using Dummy = decltype(T_StateMachine {
                std::shared_ptr<BLE_Client::ESP32_AD5933> { nullptr },
                BLE_Client::StateMachines::Logger {},
            });
        }
    }
}
