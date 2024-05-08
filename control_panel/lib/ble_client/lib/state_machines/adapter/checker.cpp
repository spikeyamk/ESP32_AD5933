#include "ble_client/state_machines/adapter/checker.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Adapter {
            void checker(
                std::stop_source stop_source,
                BLE_Client::StateMachines::Adapter::T_StateMachine& adapter_sm,
                SimpleBLE::Adapter& adapter,
                std::shared_ptr<BLE_Client::SHM::SHM> shm
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

                    if(adapter_sm.is(state<BLE_Client::StateMachines::Adapter::States::off>)) {
                        shm->active_state = BLE_Client::StateMachines::Adapter::States::off {};
                    } else if(adapter_sm.is(state<BLE_Client::StateMachines::Adapter::States::on>)) {
                        shm->active_state = BLE_Client::StateMachines::Adapter::States::on {};
                    } else if(adapter_sm.is(state<BLE_Client::StateMachines::Adapter::States::discovering>)) {
                        shm->active_state = BLE_Client::StateMachines::Adapter::States::discovering {};
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
        }
    }
}