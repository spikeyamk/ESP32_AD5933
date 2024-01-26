#include "ble_client/standalone/state_machines/connection/connection.hpp"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

namespace BLE_Client {
    namespace StateMachines {
        namespace Connection {
            namespace Actions {
                void connect(const BLE_Client::StateMachines::Connection::Events::connect& event, SimpleBLE::Adapter& adapter, SimpleBLE::Peripheral& peripheral, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
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
                        peripheral = *it;
                    } catch(const std::exception& e) {
                        std::cerr << "ERROR: BLE_Client::StateMachines::Connection::Actions::connect: exception: " << e.what() << std::endl;
                        return;
                    }
                }

                void disconnect(SimpleBLE::Peripheral& peripheral, ESP32_AD5933& esp32_ad5933) {
                    try {
                        if(peripheral.is_connected() == false) {
                            return;
                        }
                        peripheral.disconnect();
                        esp32_ad5933.remove_subscriptions();
                    } catch(const std::exception& e) {
                        std::cerr << "ERROR: BLE_Client::StateMachines::Connection::Actions::disconnect: exception: " << e.what() << std::endl;
                    }
                }

                void write(const BLE_Client::StateMachines::Connection::Events::write& event, ESP32_AD5933& esp32_ad5933) {
                    try{
                        //esp32_ad5933.write(event.packet);
                    } catch(const std::exception& e) {
                        std::cerr << "ERROR: BLE_Client::StateMachines::Connection::Actions::write: exception: " << e.what() << std::endl;
                    }
                }

                void setup_subscriptions(ESP32_AD5933& esp32_ad5933) {
                    try{
                        esp32_ad5933.setup_subscriptions();
                    } catch(const std::exception& e) {
                        std::cerr << "ERROR: BLE_Client::StateMachines::Connection::Actions::setup_subscriptions: exception: " << e.what() << std::endl;
                    }
                }
            }
            
            namespace Guards {
                bool is_connected(SimpleBLE::Peripheral& peripheral) {
                    try {
                        for(size_t i = 0; peripheral.is_connected() == false && i < 100; i++) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }

                        return peripheral.is_connected();
                    } catch(const std::exception& e) {
                        std::cerr << "ERROR: BLE_Client::StateMachines::Connection::Guards::is_connected: exception: " << e.what() << std::endl;
                        return false;
                    }
                }

                bool is_not_connected(SimpleBLE::Peripheral& peripheral) {
                    return !is_connected(peripheral);
                }

                bool is_esp32_ad5933(SimpleBLE::Peripheral& peripheral, ESP32_AD5933& esp32_ad5933, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
                    try {
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
                    } catch(const std::exception& e) {
                        std::cerr << "ERROR: BLE_Client::StateMachines::Connection::Guards::is_esp32_ad5933: exception: " << e.what() << std::endl;
                        return false;
                    }
                }

                bool is_not_esp32_ad5933(SimpleBLE::Peripheral& peripheral, ESP32_AD5933& esp32_ad5933, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
                    return !is_esp32_ad5933(peripheral, esp32_ad5933, shm);
                }
            }
        }
    }
}