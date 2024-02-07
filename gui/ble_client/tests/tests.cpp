#include <thread>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <string_view>
#include <variant>
#include <type_traits>
#include <vector>
#include <future>
#include <csignal>

#include <boost/process.hpp>
#include <boost/thread/thread_time.hpp>
#include <trielo/trielo.hpp>

#include "magic/packets/outcoming.hpp"
#include "magic/events/commands.hpp"
#include "ble_client/shm/child/child.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "ble_client/child_main.hpp"
#include "ble_client/cmd_listener.hpp"
#include "ble_client/state_machines/adapter/adapter.hpp"
#include "ble_client/state_machines/adapter/checker.hpp"
#include "ble_client/state_machines/killer/killer.hpp"
#include "ble_client/state_machines/connector/connector.hpp"
#include "ble_client/device.hpp"
#include "ad5933/config/config.hpp"

#include "ble_client/tests/tests.hpp"

namespace BLE_Client {
    namespace Tests {
        template<typename T_State>
        bool state_tester(std::shared_ptr<BLE_Client::SHM::ParentSHM> parent_shm) {
            bool success = false;
            std::visit([&](auto&& active_state) {
                using T_Decay = std::decay_t<decltype(active_state)>;
                if constexpr(std::is_same_v<T_Decay, T_State>) {
                    success = true;
                }
            }, *parent_shm->active_state);
            return success;
        }

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

        void print_dump_event_variant(const Magic::Events::Results::Variant& dump_variant) {
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

        bool dump_events_variants_are_equal(
            const Magic::Events::Results::Variant& lhs,
            const Magic::Events::Results::Variant& rhs
        ) {
            bool result = false;
            std::visit([&rhs, &result](auto&& lhs) {
                if constexpr(std::is_same_v<std::decay_t<decltype(lhs)>, Magic::Events::Results::Debug::Dump>) {
                    std::visit([&lhs, &result](auto&& rhs) {
                        if constexpr(std::is_same_v<std::decay_t<decltype(rhs)>, Magic::Events::Results::Debug::Dump>) {
                            if(lhs.registers_data == rhs.registers_data) {
                                result = true;
                            }
                        }
                    }, rhs);
                }
            }, lhs);
            return result;
        }

        bool dump_event_variant_equals_raw_data(
            const Magic::Events::Results::Variant& event_variant,
            const std::array<uint8_t, 12>& raw_register_data
        ) {
            bool result = false;
            std::visit([&raw_register_data, &result](auto&& event) {
                if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Events::Results::Debug::Dump>) {
                    if(
                        std::equal(
                            raw_register_data.begin(),
                            raw_register_data.end(),
                            event.registers_data.begin(),
                            event.registers_data.begin() + raw_register_data.size()
                        )
                    ) {
                        result = true;
                    }
                }
            }, event_variant);
            return result;
        }
    }
}

namespace BLE_Client {
    namespace Tests {
        int basic() {
            std::shared_ptr<BLE_Client::SHM::ParentSHM> parent_shm = nullptr;
            try {
                parent_shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::Tests::basic: failed to init parent_shm: exception: " << e.what() << std::endl;
                return -1;
            }

            std::thread child_thread(BLE_Client::child_main);
            child_thread.detach();

            parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::turn_on{});
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            if(state_tester<BLE_Client::StateMachines::Adapter::States::on>(parent_shm) == false) {
                return -2;
            }

            parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::start_discovery{});
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            if(state_tester<BLE_Client::StateMachines::Adapter::States::discovering>(parent_shm) == false) {
                return -3;
            }

            parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::stop_discovery{});
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            if(state_tester<BLE_Client::StateMachines::Adapter::States::on>(parent_shm) == false) {
                return -4;
            }

            parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::start_discovery{});
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            if(state_tester<BLE_Client::StateMachines::Adapter::States::discovering>(parent_shm) == false) {
                return -5;
            }

            parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::stop_discovery{});
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            if(state_tester<BLE_Client::StateMachines::Adapter::States::on>(parent_shm) == false) {
                return -6;
            }

            if(parent_shm->discovery_devices->empty()) {
                return -7;
            }

            for(int i = 1; i <= 2; i++) {
                static constexpr std::string_view nimble_address { "40:4C:CA:43:11:B2" };
                if(std::find_if(parent_shm->discovery_devices->begin(), parent_shm->discovery_devices->end(), [&](const auto& e) {
                    return nimble_address == e.get_address();
                }) == parent_shm->discovery_devices->end()) {
                    return -1 - (10 * i);
                }
                
                auto connect_event { BLE_Client::StateMachines::Connector::Events::connect{ nimble_address } };
                parent_shm->cmd.send(connect_event);
                std::this_thread::sleep_for(std::chrono::milliseconds(60'000));
                try {
                    parent_shm->attach_device(connect_event);
                } catch(...) {
                    return -2 - (10 * i);
                }

                parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ 0, Magic::Events::Commands::Debug::Start{} });
                parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ 0, Magic::Events::Commands::Debug::Dump{} });
                parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ 0, Magic::Events::Commands::Debug::End{} });

                std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
                const auto dump_all_registers { parent_shm->active_devices[0].information->read_for(boost::posix_time::milliseconds(5'000)) };
                if(dump_all_registers.has_value() == false) {
                    return -3 - (10 * i);
                }

                if(variant_tester<Magic::Events::Results::Debug::Dump>(dump_all_registers.value()) == false) {
                    return -4 - (10 * i);
                }

                print_dump_event_variant(dump_all_registers.value());

                parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::disconnect{ 0 });
                std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
                try {
                    parent_shm->active_devices.erase(parent_shm->active_devices.begin());
                } catch(const std::exception& e) {
                    std::cout << "ERROR: BLE_Client::Tests::debug_program_debug_dump: failed to erase one of the notify_channels: exception: " << e.what() << std::endl;
                    return -5 - (10 * i);
                }
            }
            
            try {
                parent_shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
                std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
                return 0;
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::Tests::basic: failed to exit from the parent because child is still running: exception: " << e.what() << std::endl;
                return -200;
            }
        }

        int debug_program_debug_dump() {
            std::shared_ptr<BLE_Client::SHM::ParentSHM> parent_shm = nullptr;
            try {
                parent_shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::Tests::debug_program_debug_dump: failed to init parent_shm: exception: " << e.what() << std::endl;
                return -1;
            }

            std::thread child_thread(BLE_Client::child_main);
            child_thread.detach();

            parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::turn_on{});
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            if(state_tester<BLE_Client::StateMachines::Adapter::States::on>(parent_shm) == false) {
                return -2;
            }

            parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::start_discovery{});
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            if(state_tester<BLE_Client::StateMachines::Adapter::States::discovering>(parent_shm) == false) {
                return -3;
            }

            parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::stop_discovery{});
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            if(state_tester<BLE_Client::StateMachines::Adapter::States::on>(parent_shm) == false) {
                return -4;
            }

            parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::start_discovery{});
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            if(state_tester<BLE_Client::StateMachines::Adapter::States::discovering>(parent_shm) == false) {
                return -5;
            }

            parent_shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::stop_discovery{});
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            if(state_tester<BLE_Client::StateMachines::Adapter::States::on>(parent_shm) == false) {
                return -6;
            }

            if(parent_shm->discovery_devices->empty()) {
                return -7;
            }

            static constexpr std::string_view nimble_address { "40:4C:CA:43:11:B2" };
            if(std::find_if(parent_shm->discovery_devices->begin(), parent_shm->discovery_devices->end(), [&](const auto& e) {
                return nimble_address == e.get_address();
            }) == parent_shm->discovery_devices->end()) {
                return -10;
            }
            
            const auto connect_event { BLE_Client::StateMachines::Connector::Events::connect{ nimble_address } };
            parent_shm->cmd.send(connect_event);
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            try {
                parent_shm->attach_device(connect_event);
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::Tests::debug_program_debug_dump: failed to attach to one of the notify_channels: exception: " << e.what() << std::endl;
                return -11;
            }

            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ 0, Magic::Events::Commands::Debug::Start{} });
            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ 0, Magic::Events::Commands::Debug::Dump{} });
            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ 0, Magic::Events::Commands::Debug::End{} });

            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            const auto dump_before { parent_shm->active_devices[0].information->read_for(boost::posix_time::milliseconds(5'000)) };
            if(dump_before.has_value() == false) {
                return -12;
            }
            if(variant_tester<Magic::Events::Results::Debug::Dump>(dump_before.value()) == false) {
                return -13;
            }
            print_dump_event_variant(dump_before.value());

            constexpr AD5933::Config default_config {
                AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode,
                AD5933::Masks::Or::Ctrl::HB::VoltageRange::Two_Vppk,
                AD5933::Masks::Or::Ctrl::HB::PGA_Gain::OneTime,
                AD5933::Masks::Or::Ctrl::LB::SysClkSrc::External,
                AD5933::uint_startfreq_t { 30'000 },
                AD5933::uint_incfreq_t { 10 },
                AD5933::uint9_t { 2 },
                AD5933::uint9_t { 15 },
                AD5933::Masks::Or::SettlingTimeCyclesHB::Multiplier::OneTime
            };
            
            const auto default_config_raw_array = default_config.to_raw_array();
            const Magic::Events::Commands::Debug::Program default_config_program_magic_event { default_config_raw_array };
            const BLE_Client::StateMachines::Connection::Events::write_body_composition_feature default_config_program_ble_client_connection_write_event { 0, default_config_program_magic_event };

            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature { 0, Magic::Events::Commands::Debug::Start{} });
            parent_shm->cmd.send(default_config_program_ble_client_connection_write_event);
            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature { 0, Magic::Events::Commands::Debug::Dump{} });
            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature { 0, Magic::Events::Commands::Debug::End{} });

            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            const auto dump_after { parent_shm->active_devices[0].information->read_for(boost::posix_time::milliseconds(5'000)) };
            if(dump_after.has_value() == false) {
                return -14;
            }
            if(variant_tester<Magic::Events::Results::Debug::Dump>(dump_after.value()) == false) {
                return -15;
            }
            std::cout << "dump_before_program:" << std::endl;
            print_dump_event_variant(dump_before.value());
            std::cout << "dump_after_program:" << std::endl;
            print_dump_event_variant(dump_after.value());
            if(dump_events_variants_are_equal(dump_before.value(), dump_after.value())) {
                return -16;
            }

            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature { 0, Magic::Events::Commands::Debug::Start{} });
            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature { 0, Magic::Events::Commands::Debug::Program{ default_config.to_raw_array() } });
            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature { 0, Magic::Events::Commands::Debug::Dump{} });
            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature { 0, Magic::Events::Commands::Debug::End{} });

            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
            const auto dump_after_after { parent_shm->active_devices[0].information->read_for(boost::posix_time::milliseconds(5'000)) };
            if(dump_after_after.has_value() == false) {
                return -17;
            }
            if(variant_tester<Magic::Events::Results::Debug::Dump>(dump_after_after.value()) == false) {
                return -18;
            }
            std::cout << "dump_before_program:" << std::endl;
            print_dump_event_variant(dump_before.value());
            std::cout << "dump_after_program:" << std::endl;
            print_dump_event_variant(dump_after.value());
            std::cout << "dump_after_after_program:" << std::endl;
            print_dump_event_variant(dump_after_after.value());
            if(dump_events_variants_are_equal(dump_after.value(), dump_after_after.value()) == false) {
                return -19;
            }
            
            if(dump_event_variant_equals_raw_data(dump_after_after.value(), default_config.to_raw_array()) == false) {
                return -20;
            }

            parent_shm->cmd.send(BLE_Client::StateMachines::Connection::Events::disconnect{ 0 });
            std::this_thread::sleep_for(std::chrono::milliseconds(5'000));

            try {
                parent_shm->active_devices.erase(parent_shm->active_devices.begin());
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::Tests::debug_program_debug_dump: failed to erase one of the notify_channels: exception: " << e.what() << std::endl;
                return -100;
            }

            try {
                parent_shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
                std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
                return 0;
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::Tests::debug_program_debug_dump: failed to exit from the parent because child is still running: exception: " << e.what() << std::endl;
                return -200;
            }
        }
    }
}

namespace BLE_Client {
    namespace Tests {
        int sync_scan_connect() {
            try {
                if(SimpleBLE::Adapter::get_adapters().empty()) {
                    return -3;
                }
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::Tests::sync_scan_connect: SimpleBLE::Adapter::get_adapters().empty() failed: exception: " << e.what() << std::endl;
                return -4;
            }

            SimpleBLE::Adapter default_adapter;
            try {
                default_adapter = SimpleBLE::Adapter::get_adapters()[0];
                default_adapter.scan_for(30'000);
                if(default_adapter.scan_get_results().empty()) {
                    return -5;
                }
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::Tests::sync_scan_connect: failed to try to initialize scan with the default adapter: exception: " << e.what() << std::endl;
                return -6;
            }

            std::vector<SimpleBLE::Peripheral> scan_results;

            try {
                scan_results = default_adapter.scan_get_results();
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::Tests::sync_scan_connect: failed to get the scan results: exception: " << e.what() << std::endl;
                return -7;
            }

            try {
                static constexpr std::string_view nimble_address { "40:4C:CA:43:11:B2" };
                auto nimble_it {
                    std::find_if(scan_results.begin(), scan_results.end(), [&](SimpleBLE::Peripheral& e) {
                        return nimble_address == e.address();
                    })
                };
                
                if(nimble_it == scan_results.end()) {
                    std::cout << "ERROR: BLE_Client::Tests::sync_scan_connect: Failed to find nimble_address device: " << nimble_address << std::endl;
                    return -8;
                }

                nimble_it->connect();
                std::this_thread::sleep_for(std::chrono::milliseconds(10'000));
                if(nimble_it->is_connected() == false) {
                    return -9;
                }

                if(find_services_characteristics(*nimble_it).has_value() == false) {
                    return -10;
                }

                nimble_it->disconnect();
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::Tests::sync_scan_connect: exception: " << e.what() << std::endl;
                return -9999;
            }
            return 0;
        }

        int async_scan_connect() {
            try {
                if(SimpleBLE::Adapter::get_adapters().empty()) {
                    return -1;
                }
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::Tests::async_scan_connect: SimpleBLE::Adapter::get_adapters().empty() failed: exception: " << e.what() << std::endl;
                return -2;
            }

            SimpleBLE::Adapter default_adapter;
            try {
                default_adapter = SimpleBLE::Adapter::get_adapters()[0];
                if(default_adapter.bluetooth_enabled() == false) {
                    return -3;
                }

                std::vector<BLE_Client::Discovery::Device> discovered_devices;
                static constexpr std::string_view nimble_address { "40:4C:CA:43:11:B2" };
                default_adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral peripheral) {
                    auto device_it {
                        std::find_if(discovered_devices.begin(), discovered_devices.end(), [&](const BLE_Client::Discovery::Device& e) {
                            return (
                                e.get_address() == peripheral.address()
                                && e.get_identifier() == peripheral.identifier()
                                && e.get_connected() == peripheral.is_connected()
                            );
                        })
                    };

                    if(device_it != discovered_devices.end()) {
                        return;
                    }

                    discovered_devices.push_back(BLE_Client::Discovery::Device{ peripheral.identifier(), peripheral.address(), peripheral.is_connected() });
                });

                default_adapter.set_callback_on_scan_updated([&](SimpleBLE::Peripheral peripheral) {
                    auto device_it {
                        std::find_if(discovered_devices.begin(), discovered_devices.end(), [&](const BLE_Client::Discovery::Device& e) {
                            return (
                                e.get_address() == peripheral.address()
                            );
                        })
                    };

                    if(device_it == discovered_devices.end()) {
                        return;
                    }

                    *device_it = BLE_Client::Discovery::Device{ peripheral.identifier(), peripheral.address(), peripheral.is_connected() };
                });

                default_adapter.scan_start();

                SimpleBLE::Peripheral nimble_peripheral;
                for(size_t i = 0; i <= 3'000; i++) {
                    static constexpr std::string_view nimble_address { "40:4C:CA:43:11:B2" };
                    auto nimble_it {
                        std::find_if(discovered_devices.begin(), discovered_devices.end(), [&](BLE_Client::Discovery::Device& e) {
                            return (
                                e.get_address() == nimble_address
                                && e.get_connected() == false
                            );
                        })
                    };
                    if(nimble_it != discovered_devices.end()) {
                        std::cout << "BLE_Client::Tests::async_scan_connect: finding NimBLE ESP32_AD5933: i: " << i << std::endl;
                        auto scan_results { default_adapter.scan_get_results() };
                        auto peripheral_it { std::find_if(scan_results.begin(), scan_results.end(), [&](SimpleBLE::Peripheral& e) {
                            return e.address() == nimble_address;
                        }) };
                        nimble_peripheral = *peripheral_it;
                        break;
                    }
                    if(i == 3'000) {
                        return -3'000;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }

                nimble_peripheral.connect();
                for(size_t i = 0; i < 3'000; i++) {
                    if(nimble_peripheral.is_connected()) {
                        std::cout << "BLE_Client::Tests::async_scan_connect: connecting to NimBLE ESP32_AD5933: i: " << i << std::endl;
                        break;
                    }
                    if(i == 3'000) {
                        return -6'000;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }

                if(find_services_characteristics(nimble_peripheral).has_value() == false) {
                    nimble_peripheral.disconnect();
                    return -10'000;
                }

                nimble_peripheral.disconnect();
                default_adapter.scan_stop();
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::Tests::async_scan_connect: failed to try to initialize scan with the default adapter: exception: " << e.what() << std::endl;
                return -4;
            }

            return 0;
        }
    }
}