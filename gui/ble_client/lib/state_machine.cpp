#include <stdexcept>
#include <simpleble/SimpleBLE.h>
#include <thread>

#include "ble_client/standalone/state_machine.hpp"
#include "ble_client/standalone/init.hpp"

namespace BLE_Client {
    namespace Discovery {
        namespace Actions {
            void turn_on() {

            }

            void default_active_adapter_found() {

            }

            void discover(SimpleBLE::Adapter& adapter) {
                adapter.scan_start();
            }

            void stop_discover(SimpleBLE::Adapter& adapter) {
                adapter.scan_stop();
                std::printf("BLE_Client::Discovery::Actions::stop_discover: adapter.scan_get_results().size(): %zu\n", adapter.scan_get_results().size());
            }

            void kill(std::stop_source stop_source) {
                stop_source.request_stop();
            }

            void connect(const BLE_Client::Discovery::Events::connect& event, SimpleBLE::Adapter& adapter, SimpleBLE::Peripheral& peripheral, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
                try {
                    std::vector<SimpleBLE::Peripheral> scan_results { adapter.scan_get_results() };
                    std::string tmp_address(event.address.begin(), event.address.end() - 1);

                    auto it = std::find_if(scan_results.begin(), scan_results.end(), [&](SimpleBLE::Peripheral& e) {
                        return tmp_address == e.address();
                    });

                    if(it == scan_results.end()) {
                        std::cerr << "ERROR: BLE_Client::Discovery::Actions::connect: didn't find the device with the address in the scan_results\n";
                        return;
                    }

                    it->connect();

                    for(size_t i = 0; it->is_connected() == false && i < 100; i++) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }

                    if(it->is_connected() == false) {
                        return;
                    }

                    peripheral = *it;
                } catch(const std::exception& e) {
                    std::cerr << "ERROR: BLE_Client::Discovery::Guards::connection_successfull: exception: " << e.what() << std::endl;
                    return;
                }
            }

            void disconnect(SimpleBLE::Peripheral& peripheral, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
                try {
                    if(peripheral.is_connected() == false) {
                        return;
                    }

                    peripheral.disconnect();
                    update_connection_status(peripheral, shm);
                } catch(const std::exception& e) {
                    std::cerr << "ERROR: BLE_Client::Discovery::Actions::disconnect: exception: " << e.what() << std::endl;
                }
            }

            void setup_subscriptions(ESP32_AD5933& esp32_ad5933) {
                esp32_ad5933.setup_subscriptions();
            }

            void remove_subscriptions(ESP32_AD5933& esp32_ad5933) {
                esp32_ad5933.remove_subscriptions();
            }

            void esp32_ad5933_write(const BLE_Client::Discovery::Events::esp32_ad5933_write& event, ESP32_AD5933& esp32_ad5933) {
                esp32_ad5933.write(event.packet); 
            }

            void update_connection_status(SimpleBLE::Peripheral& peripheral, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
                auto it = std::find_if(shm->discovery_devices->begin(), shm->discovery_devices->end(), [&](BLE_Client::Discovery::Device& device) {
                    if( (std::string(device.address.begin(), device.address.end()) == peripheral.address())
                    &&  (std::string(device.identifier.begin(), device.identifier.end()) == peripheral.identifier())) {
                        return true;
                    } else {
                        return false;
                    }
                });

                if(it != shm->discovery_devices->end()) {
                    it->connected = peripheral.is_connected();
                }
            }
        }

        namespace Guards {
            bool find_default_active_adapter(SimpleBLE::Adapter& adapter) {
                const std::optional<SimpleBLE::Adapter> tmp_adapter { BLE_Client::find_default_active_adapter() };
                if(tmp_adapter.has_value() == false) {
                    return false;
                }

                adapter = tmp_adapter.value();
                return true;
            }

            bool discovery_available(SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
                try {
                    adapter.set_callback_on_scan_start([shm]() { std::printf("BLE_Client::Scan started\n"); shm->discovery_devices->clear(); });
                    adapter.set_callback_on_scan_stop([]() { std::printf("BLE_Client::Scan stopped\n"); });
                    adapter.set_callback_on_scan_found([shm](SimpleBLE::Peripheral found_peripheral) {
                        std::printf("BLE_Client::callback_on_scan_found.\n");
                        if(std::find_if(shm->discovery_devices->begin(), shm->discovery_devices->end(), [&found_peripheral](const BLE_Client::Discovery::Device& e) {
                            return std::string(e.address.begin(), e.address.end() - 1) == found_peripheral.address();
                        }) == shm->discovery_devices->end()) {
                            shm->discovery_devices->push_back(
                                BLE_Client::Discovery::Device{
                                    found_peripheral.identifier(), 
                                    found_peripheral.address(),
                                    found_peripheral.is_connected()
                                }
                            );
                        }
                    });

                    adapter.set_callback_on_scan_updated([shm](SimpleBLE::Peripheral found_peripheral) {
                        std::printf("BLE_Client::callback_on_scan_updated.\n");
                        auto it = std::find_if(shm->discovery_devices->begin(), shm->discovery_devices->end(), [&found_peripheral](const BLE_Client::Discovery::Device& e) {
                            return std::string(e.address.begin(), e.address.end() - 1) == found_peripheral.address();
                        });

                        if(it == shm->discovery_devices->end()) {
                            shm->discovery_devices->push_back(
                                BLE_Client::Discovery::Device{
                                    found_peripheral.identifier(), 
                                    found_peripheral.address(),
                                    found_peripheral.is_connected()
                                }
                            );
                        } else {
                            *it = BLE_Client::Discovery::Device {
                                found_peripheral.identifier(), 
                                found_peripheral.address(),
                                found_peripheral.is_connected()
                            };
                        }
                    });
                } catch(const std::exception& e) {
                    std::cerr << "ERROR: BLE_Client::Discovery::Guards::discovery_available: exception: " << e.what() << std::endl;
                    return false;
                }

                return true;
            }

            bool discovered_at_least_one(SimpleBLE::Adapter& adapter) {
                if(adapter.scan_get_results().size() > 0) {
                    return true;
                } else {
                    return false;
                }
            }

            bool discovered_none(SimpleBLE::Adapter& adapter) {
                return !discovered_at_least_one(adapter);
            }

            bool is_connected(SimpleBLE::Peripheral& peripheral) {
                try {
                    return peripheral.is_connected();
                } catch(const std::exception& e) {
                    std::cerr << "ERROR: CLE_Client::Discovery::Guards::is_connected: exception: " << e.what() << std::endl;
                    return false;
                }
            }

            bool is_not_connected(SimpleBLE::Peripheral& peripheral) {
                return is_connected(peripheral);
            }

            bool is_esp32_ad5933(SimpleBLE::Peripheral& peripheral, ESP32_AD5933& esp32_ad5933, BLE_Client::SHM::SHM* shm) {
                const std::string BODY_COMPOSITION_SERVICE_UUID { "0000181b-0000-1000-8000-00805f9b34fb" };
                const std::string BODY_COMPOSITION_FEATURE_UUID { "00002a9b-0000-1000-8000-00805f9b34fb" };
                const std::string BODY_COMPOSITION_MEASUREMENT_UUID { "00002a9c-0000-1000-8000-00805f9b34fb" };

                const auto is_characteristic_body_composition_measurement = [&](SimpleBLE::Characteristic &characteristic) {
                    if(characteristic.uuid() == BODY_COMPOSITION_MEASUREMENT_UUID &&
                    (characteristic.can_indicate() && characteristic.can_notify()))
                    {
                        return true;
                    } else {
                        return false;
                    }
                };

                const auto is_characteristic_body_composition_feature = [&](SimpleBLE::Characteristic &characteristic) {
                    if(characteristic.uuid() == BODY_COMPOSITION_FEATURE_UUID &&
                    (characteristic.can_write_request()) 
                    ) {
                        return true;
                    } else {
                        return false;
                    }
                };

                auto services { peripheral.services() };
                auto it_service = std::find_if(services.begin(), services.end(), [&](SimpleBLE::Service& e) {
                    return e.uuid() == BODY_COMPOSITION_SERVICE_UUID;
                });
                if(it_service == services.end()) {
                    return false;
                }

                auto characteristics { it_service->characteristics() };
                auto it_body_composition_measurement = std::find_if(characteristics.begin(), characteristics.end(), [&](SimpleBLE::Characteristic& e) {
                    return is_characteristic_body_composition_measurement(e);
                });
                if(it_body_composition_measurement == characteristics.end()) {
                    return false;
                }

                auto it_body_composition_feature = std::find_if(characteristics.begin(), characteristics.end(), [&](SimpleBLE::Characteristic& e) {
                    return is_characteristic_body_composition_feature(e);
                });
                if(it_body_composition_feature == characteristics.end()) {
                    return false;
                }

                ESP32_AD5933 tmp_ret { peripheral, *it_service, *it_body_composition_measurement, *it_body_composition_feature, shm };
                esp32_ad5933 = tmp_ret;
                return true;
            }
        }
    }
}
