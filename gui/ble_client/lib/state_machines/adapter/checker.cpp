#include "ble_client/standalone/state_machines/adapter/checker.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Adapter {
            void checker(
                std::stop_source stop_source,
                BLE_Client::StateMachines::Adapter::T_StateMachine& adapter_sm,
                SimpleBLE::Adapter& adapter,
                std::shared_ptr<BLE_Client::SHM::ChildSHM> shm
            ) {
                std::stop_token st = stop_source.get_token();
                using namespace boost::sml;
                while(st.stop_requested() == false) {
                    if(adapter_sm.is(state<BLE_Client::StateMachines::Adapter::States::off>) == false) {
                        try {
                            if(adapter.bluetooth_enabled() == false) {
                                adapter_sm.set_current_states(state<BLE_Client::StateMachines::Adapter::States::off>);
                            }
                        } catch(const std::exception& e) {
                            shm->console.log(std::string("ERROR: BLE_Client::checker: exception: ") + e.what() + "\n");
                            adapter_sm.set_current_states(state<BLE_Client::StateMachines::Adapter::States::off>);
                        }
                    }

                    adapter_sm.visit_current_states([&](auto&& visited_state) {
                        try {
                            *(shm->active_state) = BLE_Client::StateMachines::Adapter::States::stupid_sml.at(visited_state.c_str());
                        } catch(const std::exception& e) {
                            shm->console.log(std::string("ERROR: BLE_Client::checker: exception: ") + e.what() + "\n");
                            shm->console.log("ERROR: This compiler gives the visited_state a different prefix\n");
                            std::exit(-1);
                        }
                    });
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }
    }
}