#include "ble_client/standalone/state_machines/adapter/adapter.hpp"
#include "ble_client/standalone/init.hpp"

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
                bool bluetooth_active(SimpleBLE::Adapter& adapter) {
                    const std::optional<SimpleBLE::Adapter> tmp_adapter { BLE_Client::find_default_active_adapter() };
                    if(tmp_adapter.has_value() == false) {
                        return false;
                    }

                    adapter = tmp_adapter.value();
                    return true;
                }

                bool discovery_available(SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm) {
                    try {
                        adapter.set_callback_on_scan_start([shm]() {
                            std::printf("BLE_Client::Scan started\n");
                            shm->discovery_devices->clear();
                        });
                        adapter.set_callback_on_scan_stop([]() { std::printf("BLE_Client::Scan stopped\n"); });
                        adapter.set_callback_on_scan_found([shm](SimpleBLE::Peripheral found_peripheral) {
                            std::printf("BLE_Client::callback_on_scan_found.\n");
                            if(std::find_if(shm->discovery_devices->begin(), shm->discovery_devices->end(), [&found_peripheral](const BLE_Client::Discovery::Device& e) {
                                std::printf("BLE_Client::callback_on_scan_found: inside std::find_if\n");
                                return e.get_address() == found_peripheral.address();
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
                                return e.get_address() == found_peripheral.address();
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
                        return true;
                    } catch(const std::exception& e) {
                        std::cerr << "ERROR: BLE_Client::Discovery::Guards::discovery_available: exception: " << e.what() << std::endl;
                        return false;
                    }
                }
            }
        }
    }
}