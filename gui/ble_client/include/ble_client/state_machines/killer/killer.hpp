#pragma once

#include <mutex>
#include <stop_token>
#include <functional>

#include <boost/sml.hpp>

#include "ble_client/state_machines/logger.hpp"
#include "ble_client/state_machines/killer/events.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Killer {
            namespace States {
                struct alive{};
                struct dead{};
            }

            namespace Actions {
                void kill(std::stop_source stop_source);
            }
            
            struct Killer {
                auto operator()() const {
                    using namespace boost::sml;
                    using namespace std;
                    auto ret = make_transition_table(
                        *state<States::alive> + event<Events::kill> / function{Actions::kill} = state<States::dead>
                    );
                    return ret;
                }
            };
            using T_StateMachine = boost::sml::sm<Killer, boost::sml::logger<BLE_Client::StateMachines::Logger>, boost::sml::thread_safe<std::recursive_mutex>, boost::sml::testing>;
        }
    }
}