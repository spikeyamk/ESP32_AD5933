#include <memory>
#include <variant>
#include <string_view>
#include <thread>
#include <chrono>

#include <trielo/trielo.hpp>
#include <simpleble/SimpleBLE.h>
#include <boost/process.hpp>

#include "ble_client/esp32_ad5933.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "ble_client/shm/child/child.hpp"
#include "ble_client/child_main.hpp"
#include "ble_client/state_machines/adapter/states.hpp"

#include "ble_client/tests/tests.hpp"

namespace BLE_Client {
    namespace Tests {
        template<typename T_Decay>
        bool variant_tester(const auto& variant) {
            bool result = false;
            std::visit([&result](auto&& active_state) {
                if constexpr(std::is_same_v<std::decay_t<decltype(active_state)>, T_Decay>) {
                    result = true;
                }
            }, variant);
            return result;
        }

        namespace Debug {
            static void print_dump_event_variant(const Magic::Events::Results::Variant& dump_variant) {
                std::visit([](auto&& dump) {
                    if constexpr(std::is_same_v<std::decay_t<decltype(dump)>, Magic::Events::Results::Debug::Dump>) {
                        std::printf("BLE_Client::basic_test: dump_all_registers:");
                        std::for_each(dump.registers_data.begin(), dump.registers_data.end(), [index = 0](const auto e) mutable {
                            if(index % 8 == 0) {
                                std::printf("\n    ");
                            }
                            std::printf("0x%02X, ", e);
                            index++;
                        });
                        std::printf("\n");
                    }
                }, dump_variant);
            }

            int interprocess_dump(const char* self_path, const std::string_view& magic_key) {
                std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };
                try {
                    shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
                } catch(const boost::interprocess::interprocess_exception& e) {
                    std::cout << "ERROR: BLE_Client::Tests::Connect::interprocess_sync_scan: Failed to open SHM: exception: " << e.what() << std::endl;
                    std::cout << "Try killing all child processes and cleaning with shm_cleaner\n";
                    return -1;
                }

                boost::process::child child(self_path, magic_key.data());
                std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
                if(child.running() == false) {
                    return -2;
                }


                shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::turn_on{});
                std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
                if(variant_tester<BLE_Client::StateMachines::Adapter::States::on>(*shm->active_state) == false) {
                    return -2;
                }

                shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::start_discovery{});
                std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
                if(variant_tester<BLE_Client::StateMachines::Adapter::States::discovering>(*shm->active_state) == false) {
                    return -3;
                }

                if(shm->discovery_devices->empty()) {
                    return -5;
                }

                static constexpr std::string_view nimble_address {
                    #ifdef _MSC_VER
                    "40:4c:ca:43:11:b2"
                    #else
                    "40:4C:CA:43:11:B2"
                    #endif
                };

                for(int i = 0; i < 3'000; i++) {
                    if(std::find_if(shm->discovery_devices->begin(), shm->discovery_devices->end(), [&](const auto& e) {
                        return nimble_address == e.get_address();
                    }) == shm->discovery_devices->end()) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    } else {
                        break;
                    }
                    if(i == 2'999) {
                        return i;
                    }
                }

                const auto connect_event { BLE_Client::StateMachines::Connector::Events::connect{ nimble_address } };
                shm->cmd.send(connect_event);
                std::this_thread::sleep_for(std::chrono::milliseconds(10'000));
                try {
                    shm->attach_device(connect_event);
                } catch(...) {
                    return -7;
                }

                shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ 0, Magic::Events::Commands::Debug::Start{} });
                shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ 0, Magic::Events::Commands::Debug::Dump{} });
                shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ 0, Magic::Events::Commands::Debug::End{} });

                std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
                const auto dump { shm->active_devices[0].information->read_for(boost::posix_time::milliseconds(5'000)) };
                if(dump.has_value() == false) {
                    return -12;
                }
                if(variant_tester<Magic::Events::Results::Debug::Dump>(dump.value()) == false) {
                    return -13;
                }
                
                print_dump_event_variant(dump.value());

                shm->cmd.send(BLE_Client::StateMachines::Killer::Events::kill{});
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                if(child.running()) {
                    child.terminate();
                    return -14;
                }

                return 0;
            }

            int child_main() {
                return BLE_Client::child_main();
            }
       }
    }
}

int main(int argc, char* argv[]) {
    static constexpr std::string_view magic_key { "okOvDRmWjEUErr3grKvWKpHw2Z0c8L5p6rjl5KT4HAoRGenjFFdPxegc43vCt8BR9ZdWJIPiaMaTYwhr6TMu4od0gHO3r1f7qTQ8pmaQtEm12SqT3IikKLdAsAI46N9E" };
    if(argc > 1 && argv[1] == magic_key) {
        return Trielo::trielo<BLE_Client::Tests::Debug::child_main>(Trielo::OkErrCode(0));
    }
    return Trielo::trielo<BLE_Client::Tests::Debug::interprocess_dump>(Trielo::OkErrCode(0), argv[0], magic_key);
}