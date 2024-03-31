#pragma once

#include <mutex>
#include <memory>
#include <functional>

#include <boost/sml.hpp>
#include <simpleble/SimpleBLE.h>

#include "ble_client/state_machines/logger.hpp"
#include "ble_client/state_machines/adapter/events.hpp"
#include "ble_client/state_machines/adapter/states.hpp"
#include "ble_client/shm/parent/parent.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Adapter {
            namespace Actions {
                void start_discovery(SimpleBLE::Adapter& adapter);
                void stop_discovery(SimpleBLE::Adapter& adapter);
            }
            
            namespace Guards {
                bool bluetooth_active(SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::Parent> shm);
                bool discovery_available(SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::Parent> shm);
            }

            struct Adapter {
                auto operator()() const {
                    using namespace boost::sml;
                    using namespace std;
                    auto ret = make_transition_table(
                        *state<States::off> + event<Events::turn_on> [function{Guards::bluetooth_active}] = state<States::on>,
                        state<States::on> + event<Events::start_discovery> [function{Guards::discovery_available}] / function{Actions::start_discovery} = state<States::discovering>,
                        state<States::discovering> + event<Events::stop_discovery> / function{Actions::stop_discovery} = state<States::on>
                    );
                    return ret;
                }
            };
            using T_StateMachine = boost::sml::sm<Adapter, boost::sml::logger<BLE_Client::StateMachines::Logger>, boost::sml::thread_safe<std::recursive_mutex>, boost::sml::testing>;
        }
    }
}