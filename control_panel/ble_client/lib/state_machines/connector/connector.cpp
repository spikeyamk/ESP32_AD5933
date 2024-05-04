#include "ble_client/state_machines/connector/connector.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Connector {
            namespace Guards {
                bool successful(const BLE_Client::StateMachines::Connector::Events::connect& event, SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::SHM> shm, std::vector<BLE_Client::StateMachines::Connection::Dummy*>& connections) {
                    try {
                        std::vector<SimpleBLE::Peripheral> scan_results { adapter.scan_get_results() };
                        auto it = std::find_if(scan_results.begin(), scan_results.end(), [&](SimpleBLE::Peripheral& e) {
                            return event.get_address() == e.address();
                        });

                        if(it == scan_results.end()) {
                            shm->console.log("ERROR: BLE_Client::Discovery::Actions::connect: didn't find the device with the address in the scan_results\n");
                            return false;
                        }

                        it->connect();
                        for(size_t i = 0, timeout_ms = 5'000; it->is_connected() == false && i <= timeout_ms; i++) {
                            if(i == timeout_ms) {
                                it->disconnect();
                                throw std::runtime_error("timed out");
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                        
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

                bool failed(const BLE_Client::StateMachines::Connector::Events::connect& event, SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::SHM> shm, std::vector<BLE_Client::StateMachines::Connection::Dummy*>& connections) {
                    return !successful(event, adapter, shm, connections);
                }
            }
        }
    }
}