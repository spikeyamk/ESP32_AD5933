#include "ble_client/ostream_overloads.hpp"
#include "ble_client/esp32_ad5933.hpp"
#include "ble_client/init.hpp"

#include "ble_client/state_machines/adapter/adapter.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Adapter {
            namespace States {

            }

            namespace Events {

            }

            namespace Actions {
                void start_discovery(SimpleBLE::Adapter& adapter) {
                    adapter.scan_start();
                }

                void stop_discovery(SimpleBLE::Adapter& adapter) {
                    adapter.scan_stop();
                }
            }
            
            namespace Guards {
                bool connect(const Events::connect& event, SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::SHM> shm, std::vector<BLE_Client::StateMachines::Connection::Dummy*>& connections) {
                    try {
                        std::vector<SimpleBLE::Peripheral> scan_results { adapter.scan_get_results() };
                        auto it { std::find_if(scan_results.begin(), scan_results.end(), [&](SimpleBLE::Peripheral& e) {
                            return event.get_address() == e.address();
                        }) };

                        if(it == scan_results.end()) {
                            shm->console.log("ERROR: BLE_Client::Discovery::Actions::connect: didn't find the device with the address in the scan_results\n");
                            return false;
                        }

                        it->connect();
                        for(size_t i = 0, timeout_ms = 1'000; it->is_connected() == false && i <= timeout_ms; i++) {
                            if(i == timeout_ms) {
                                it->disconnect();
                                throw std::runtime_error("timed out");
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                        
                        std::optional<BLE_Client::ESP32_AD5933::Service> service { find_services_characteristics(*it) };
                        if(service.has_value() == false) {
                            it->disconnect();
                            throw std::runtime_error("not an ESP32_AD5933");
                        }

                        shm->attach_device(Events::connect{ it->address() });
                        auto tmp_esp32_ad5933 { std::make_shared<BLE_Client::ESP32_AD5933>(*it, service.value(), shm->active_devices->back().measurement, shm->active_devices->back().information, shm) };
                        tmp_esp32_ad5933->setup_subscriptions();
                        tmp_esp32_ad5933->update_time();
                        BLE_Client::StateMachines::Logger logger {};
                        connections.push_back(new BLE_Client::StateMachines::Connection::Dummy { tmp_esp32_ad5933, logger, shm });
                        auto discovery_devices_update_it { std::find_if(shm->discovery_devices.begin(), shm->discovery_devices.end(), [&it](const BLE_Client::Discovery::Device& e) {
                            return e.get_address() == it->address();
                        }) };

                        if(discovery_devices_update_it != shm->discovery_devices.end()) {
                            const BLE_Client::Discovery::Device tmp_device { it->identifier(), it->address(), true };
                            *discovery_devices_update_it = tmp_device;
                        }
                        return true;
                    } catch(const std::exception& e) {
                        shm->console.log(std::string("ERROR: BLE_Client::StateMachines::Connector::Actions::connect: exception: ") + e.what() + "\n");
                        return false;
                    }
                }

                bool turn_on_successful(SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
                    const std::optional<SimpleBLE::Adapter> tmp_adapter { BLE_Client::find_default_active_adapter(shm) };
                    if(tmp_adapter.has_value() == false) {
                        return false;
                    }

                    adapter = tmp_adapter.value();
                    return true;
                }

                static bool is_esp32_ad5933_before_attempting_connect(SimpleBLE::Peripheral& peripheral) {
                    for(const auto& e: peripheral.manufacturer_data()) {
                        static constexpr uint16_t manufacturer_data_first_two_bytes_magic_dont_ask_dont_tell_its_terrible_i_hate_this_abusing_manufacturer_data_advertising_field = 26990;
                        if(e.first == manufacturer_data_first_two_bytes_magic_dont_ask_dont_tell_its_terrible_i_hate_this_abusing_manufacturer_data_advertising_field) {
                            return true;
                        }
                    }

                    return false;
                }

                bool discovery_available(SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
                    try {
                        adapter.set_callback_on_scan_start([shm]() {
                            shm->console.log("BLE_Client::callback_on_scan_start\n");
                            auto remove_it { std::remove_if(shm->discovery_devices.begin(), shm->discovery_devices.end(), [](const auto& e) {
                                if(e.get_connected() == false) {
                                    return true;
                                } else {
                                    return false;
                                }
                            }) };
                            if(remove_it != shm->discovery_devices.end()) {
                                shm->discovery_devices.erase(remove_it);
                            }
                        });
                        adapter.set_callback_on_scan_stop([shm]() { shm->console.log("BLE_Client::Scan stopped\n"); });
                        adapter.set_callback_on_scan_found([shm](SimpleBLE::Peripheral found_peripheral) {
                            if(std::find_if(shm->discovery_devices.begin(), shm->discovery_devices.end(), [&found_peripheral, shm](const BLE_Client::Discovery::Device& e) {
                                return e.get_address() == found_peripheral.address();
                            }) == shm->discovery_devices.end()) {
                                if(is_esp32_ad5933_before_attempting_connect(found_peripheral)) {
                                    for(auto service: found_peripheral.services()) {
                                        std::cout << service << std::endl;
                                    }

                                    shm->discovery_devices.push_back(
                                        BLE_Client::Discovery::Device{
                                            found_peripheral.identifier(), 
                                            found_peripheral.address(),
                                            found_peripheral.is_connected()
                                        }
                                    );
                                }
                            }
                        });
                        
                        adapter.set_callback_on_scan_updated([shm](SimpleBLE::Peripheral found_peripheral) {
                            auto it = std::find_if(shm->discovery_devices.begin(), shm->discovery_devices.end(), [&found_peripheral, shm](const BLE_Client::Discovery::Device& e) {
                                return e.get_address() == found_peripheral.address();
                            });

                            if(it == shm->discovery_devices.end()) {
                                if(is_esp32_ad5933_before_attempting_connect(found_peripheral)) {
                                    for(auto service: found_peripheral.services()) {
                                        std::cout << service << std::endl;
                                    }

                                    shm->discovery_devices.push_back(
                                        BLE_Client::Discovery::Device{
                                            found_peripheral.identifier(), 
                                            found_peripheral.address(),
                                            found_peripheral.is_connected()
                                        }
                                    );
                                }
                            } else {
                                *it = BLE_Client::Discovery::Device {
                                    found_peripheral.identifier(), 
                                    found_peripheral.address(),
                                    found_peripheral.is_connected()
                                };
                            }
                        });
                        return true;
                    } catch(const std::exception& e) {
                        shm->console.log(std::string("ERROR: BLE_Client::Discovery::Guards::discovery_available: exception: ") + e.what() + "\n");
                        return false;
                    }
                }
            }
        }
    }
}