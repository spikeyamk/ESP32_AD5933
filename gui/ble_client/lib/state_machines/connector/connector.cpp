#include "ble_client/state_machines/connector/connector.hpp"

#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>
#include <memory>

namespace BLE_Client {
    namespace StateMachines {
        namespace Connector {
            namespace Guards {
                bool successful(const BLE_Client::StateMachines::Connector::Events::connect& event, SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm, std::vector<decltype(BLE_Client::StateMachines::Connection::Dummy<int>::sm)*>& connections) {
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

                        for(size_t i = 0; it->is_connected() == false && i <= 100; i++) {
                            if(i == 100) {
                                throw std::runtime_error("timed out");
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                        
                        auto ret { find_services_characteristics(*it) };
                        if(ret.has_value() == false) {
                            it->disconnect();
                            throw std::runtime_error("not an ESP32_AD5933");
                        }

                        shm->init_device(Events::connect{ it->address() });
                        auto tmp_esp32_ad5933 { std::make_shared<BLE_Client::ESP32_AD5933>(*it, ret.value(), shm->active_devices.back().measurement, shm->active_devices.back().information, shm) };
                        tmp_esp32_ad5933->setup_subscriptions();
                        //tmp_esp32_ad5933->update_time();
                        BLE_Client::StateMachines::Logger logger {};
                        connections.push_back(new decltype(BLE_Client::StateMachines::Connection::Dummy<int>::sm){ tmp_esp32_ad5933, logger, shm });
                        return true;
                    } catch(const std::exception& e) {
                        shm->console.log(std::string("ERROR: BLE_Client::StateMachines::Connector::Actions::connect: exception: ") + e.what() + "\n");
                        shm->console.log("ERROR: BLE_Client::Discovery::Actions::connect: didn't find the device with the address in the scan_results\n");
                        return false;
                    }
                }

                bool failed(const BLE_Client::StateMachines::Connector::Events::connect& event, SimpleBLE::Adapter& adapter, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm, std::vector<decltype(BLE_Client::StateMachines::Connection::Dummy<int>::sm)*>& connections) {
                    return successful(event, adapter, shm, connections);
                }
            }
        }
    }
}