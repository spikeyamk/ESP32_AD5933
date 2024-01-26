#pragma once

#include <functional>
#include <mutex>
#include <variant>
#include <memory>
#include <stop_token>
#include <string>

#include <boost/sml.hpp>
#include <simpleble/SimpleBLE.h>

#include "ble_client/standalone/events.hpp"
#include "ble_client/standalone/states.hpp"
#include "ble_client/standalone/shm.hpp"
#include "ble_client/standalone/esp32_ad5933.hpp"
#include "ble_client/standalone/state_machines/logger.hpp"

namespace BLE_Client {
    namespace Discovery {
        namespace Actions {
            void turn_on();
            void default_active_adapter_found();
            void discover(SimpleBLE::Adapter& adapter);
            void stop_discover(SimpleBLE::Adapter& adapter);
            void kill(std::stop_source stop_source);
            void connect(const BLE_Client::Discovery::Events::connect& event, SimpleBLE::Adapter& adapter, SimpleBLE::Peripheral& peripheral, std::shared_ptr<BLE_Client::SHM::SHM> shm);
            void disconnect(SimpleBLE::Peripheral& peripheral, std::shared_ptr<BLE_Client::SHM::SHM> shm);
            void setup_subscriptions(ESP32_AD5933& esp32_ad5933);
            void remove_subscriptions(ESP32_AD5933& esp32_ad5933);
            void esp32_ad5933_write(const BLE_Client::Discovery::Events::esp32_ad5933_write& event, ESP32_AD5933& esp32_ad5933);
            void update_connection_status(SimpleBLE::Peripheral& peripheral, std::shared_ptr<BLE_Client::SHM::SHM> shm);
        }

        namespace Guards {
            bool find_default_active_adapter(SimpleBLE::Adapter& adapter);
            bool discovery_available(SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::SHM> shm);
            bool discovered_at_least_one(SimpleBLE::Adapter& adapter);
            bool discovered_none(SimpleBLE::Adapter& adapter);
            bool is_connected(SimpleBLE::Peripheral& peripheral);
            bool is_not_connected(SimpleBLE::Peripheral& peripheral);
            bool is_esp32_ad5933(SimpleBLE::Peripheral& peripheral, ESP32_AD5933& esp32_ad5933, std::shared_ptr<BLE_Client::SHM::SHM> shm);
        }

        struct Discover {
            auto operator()() const {
                using namespace boost::sml;
                using namespace std;
                auto ret = make_transition_table(
                    *state<States::off> + event<Events::find_default_active_adapter> [function{Guards::find_default_active_adapter}] / function{Actions::default_active_adapter_found} = state<States::using_adapter>,
                    state<States::using_adapter> + event<Events::start_discovery> [function{Guards::discovery_available}] / function{Actions::discover} = state<States::discovering>,

                    state<States::discovering> + event<Events::stop_discovery> [function{Guards::discovered_at_least_one}] / function{Actions::stop_discover} = state<States::discovered>,
                    state<States::discovering> + event<Events::stop_discovery> [function{Guards::discovered_none}] / function{Actions::stop_discover} = state<States::using_adapter>,

                    state<States::discovered> + event<Events::connect> / function{Actions::connect} = state<States::connecting>,
                    state<States::discovered> + event<Events::start_discovery> [function{Guards::discovery_available}] / function{Actions::discover} = state<States::discovering>,

                    state<States::connecting> + event<Events::is_connected> [function{Guards::is_connected}] / function{Actions::update_connection_status} = state<States::connected>,

                    state<States::connected> + event<Events::disconnect> / function{Actions::disconnect} = state<States::discovered>,
                    state<States::connected> + event<Events::is_esp32_ad5933> [function{Guards::is_esp32_ad5933}] / function{Actions::setup_subscriptions} = state<States::using_esp32_ad5933>,

                    state<States::using_esp32_ad5933> + event<Events::stop_using_esp32_ad5933> / function{Actions::remove_subscriptions} = state<States::connected>,
                    state<States::using_esp32_ad5933> + event<Events::esp32_ad5933_write> / function{Actions::esp32_ad5933_write} = state<States::using_esp32_ad5933>,
                    *state<States::alive> + event<Events::kill> / function{Actions::kill} = state<States::dead>
                );
                return ret;
            }
        };

        using T_StateMachine = boost::sml::sm<Discover, boost::sml::logger<BLE_Client::StateMachines::Logger>, boost::sml::thread_safe<std::recursive_mutex>, boost::sml::testing>;
        using T_StateMachineExpanded = boost::ext::sml::v1_1_9::back::sm<
            boost::ext::sml::v1_1_9::back::sm_policy<
                BLE_Client::Discovery::Discover,
                boost::ext::sml::v1_1_9::back::policies::logger<BLE_Client::StateMachines::Logger>,
                boost::ext::sml::v1_1_9::back::policies::thread_safe<std::recursive_mutex>,
                boost::ext::sml::v1_1_9::back::policies::testing
            >
        >;
    }
}
