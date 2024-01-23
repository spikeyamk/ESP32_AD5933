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

namespace BLE_Client {
    namespace Discovery {
        namespace Actions {
            void turn_on();
            void default_active_adapter_found();
            void discover(SimpleBLE::Adapter& adapter);
            void stop_discover(SimpleBLE::Adapter& adapter);
            void kill(std::stop_source stop_source);
            void connect(const BLE_Client::Discovery::Events::connect& event, SimpleBLE::Adapter& adapter, SimpleBLE::Peripheral& peripheral);
            void disconnect(SimpleBLE::Peripheral& peripheral);
            void setup_subscriptions(ESP32_AD5933& esp32_ad5933);
            void remove_subscriptions(ESP32_AD5933& esp32_ad5933);
            void esp32_ad5933_write(const BLE_Client::Discovery::Events::esp32_ad5933_write& event, ESP32_AD5933& esp32_ad5933);
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

        struct my_logger {
            // https://github.com/boost-ext/sml/blob/master/example/logging.cpp
            template <class SM, class TEvent>
            void log_process_event(const TEvent&) {
                std::printf("[%s][process_event] %s\n",
                    boost::sml::aux::get_type_name<SM>(),
                    boost::sml::aux::get_type_name<TEvent>()
                );
            }

            template <class SM, class TGuard, class TEvent>
            void log_guard(const TGuard&, const TEvent&, bool result) {
                std::printf("[%s][guard] %s %s %s\n",
                    boost::sml::aux::get_type_name<SM>(),
                    boost::sml::aux::get_type_name<TGuard>(),
                    boost::sml::aux::get_type_name<TEvent>(), (result ? "[OK]" : "[Reject]")
                );
            }

            template <class SM, class TAction, class TEvent>
            void log_action(const TAction&, const TEvent&) {
                std::printf("[%s][action] %s %s\n",
                    boost::sml::aux::get_type_name<SM>(),
                    boost::sml::aux::get_type_name<TAction>(),
                    boost::sml::aux::get_type_name<TEvent>()
                );
            }

            template <class SM, class TSrcState, class TDstState>
            void log_state_change(const TSrcState& src, const TDstState& dst) {
                std::printf("[%s][transition] %s -> %s\n",
                    boost::sml::aux::get_type_name<SM>(),
                    src.c_str(),
                    dst.c_str()
                );
            }
        };

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

                    state<States::connecting> + event<Events::is_connected> [function{Guards::is_connected}] = state<States::connected>,
                    state<States::connecting> + event<Events::is_connected> [function{Guards::is_not_connected}] = state<States::discovered>,

                    state<States::connected> + event<Events::disconnect> / function{Actions::disconnect} = state<States::discovered>,
                    state<States::connected> + event<Events::is_esp32_ad5933> [function{Guards::is_esp32_ad5933}] / function{Actions::setup_subscriptions} = state<States::using_esp32_ad5933>,

                    state<States::using_esp32_ad5933> + event<Events::stop_using_esp32_ad5933> / function{Actions::remove_subscriptions} = state<States::connected>,
                    state<States::using_esp32_ad5933> + event<Events::esp32_ad5933_write> / function{Actions::esp32_ad5933_write} = state<States::using_esp32_ad5933>,
                    *state<States::alive> + event<Events::kill> / function{Actions::kill} = state<States::dead>
                );
                return ret;
            }
        };

        using T_StateMachine = boost::sml::sm<Discover, boost::sml::logger<my_logger>, boost::sml::thread_safe<std::recursive_mutex>, boost::sml::testing>;
    }
}
