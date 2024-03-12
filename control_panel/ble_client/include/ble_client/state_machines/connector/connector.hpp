#pragma once

#include <functional>
#include <mutex>
#include <vector>
#include <memory>
#include <boost/sml.hpp>
#include <simpleble/SimpleBLE.h>

#include "ble_client/state_machines/connector/events.hpp"
#include "ble_client/state_machines/logger.hpp"
#include "ble_client/state_machines/connection/connection.hpp"
#include "ble_client/shm/child/child.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Connector {
            namespace States {
                struct off{};
                struct connected{};
                struct failed{};
            }

            namespace Actions {

            }

            namespace Guards {
                bool successful(const BLE_Client::StateMachines::Connector::Events::connect& event, SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm, std::vector<decltype(BLE_Client::StateMachines::Connection::Dummy<int>::sm)*>& connections);
                bool failed(const BLE_Client::StateMachines::Connector::Events::connect& event, SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm, std::vector<decltype(BLE_Client::StateMachines::Connection::Dummy<int>::sm)*>& connections);
            }

            struct Connector {
                auto operator()() const {
                    using namespace boost::sml;
                    using namespace std;
                    auto ret = make_transition_table(
                        *state<States::off> = state<States::failed>,
                        state<States::failed> + event<Events::connect> [function{Guards::successful}] = state<States::connected>,
                        state<States::failed> + event<Events::connect> [function{Guards::failed}] = state<States::failed>,
                        state<States::connected> + event<Events::connect> [function{Guards::successful}] = state<States::connected>,
                        state<States::connected> + event<Events::connect> [function{Guards::failed}] = state<States::failed>
                    );
                    return ret;
                }
            };

            using T_StateMachine = boost::sml::sm<Connector, boost::sml::logger<BLE_Client::StateMachines::Logger>, boost::sml::thread_safe<std::recursive_mutex>, boost::sml::testing>;
        }
    }
}