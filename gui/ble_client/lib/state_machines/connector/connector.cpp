#include "ble_client/standalone/state_machines/connector/connector.hpp"

#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>

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
                            std::cerr << "ERROR: BLE_Client::Discovery::Actions::connect: didn't find the device with the address in the scan_results\n";
                            return false;
                        }

                        it->connect();

                        for(size_t i = 0; it->is_connected() == false && i <= 100; i++) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            if(i == 100) {
                                throw std::runtime_error("timed out");
                            }
                        }
                        
                        auto ret { find_services_characteristics(*it) };
                        if(ret.has_value() == false) {
                            it->disconnect();
                            throw std::runtime_error("not an ESP32_AD5933");
                        }

                        shm->init_notify_channel(get_address_without_semicolons(*it).c_str());
                        BLE_Client::ESP32_AD5933 tmp_esp32_ad5933 { *it, std::get<0>(ret.value()), std::get<1>(ret.value()), std::get<2>(ret.value()), shm->notify_channels.back() };
                        tmp_esp32_ad5933.setup_subscriptions();
                        BLE_Client::StateMachines::Logger logger;
                        connections.push_back(new decltype(BLE_Client::StateMachines::Connection::Dummy<int>::sm){ tmp_esp32_ad5933, logger });
                        return true;
                    } catch(const std::exception& e) {
                        std::cerr << "ERROR: BLE_Client::StateMachines::Connector::Actions::connect: exception: " << e.what() << std::endl;
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