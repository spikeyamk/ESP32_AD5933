#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
#include <string_view>

#include "magic/commands/serializer.hpp"

#include "ble_client/state_machines/connection/connection.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Connection {
            namespace Actions {
                void disconnect(std::shared_ptr<ESP32_AD5933> esp32_ad5933, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
                    try {
                        if(esp32_ad5933->is_connected() == false) {
                            return;
                        }
                        esp32_ad5933->remove_subscriptions();
                        esp32_ad5933->disconnect();

                        const BLE_Client::Discovery::Device tmp_device { esp32_ad5933->peripheral.identifier(), esp32_ad5933->peripheral.address(), false };
                        auto discovery_devices_update_it { std::find_if(shm->discovery_devices.begin(), shm->discovery_devices.end(), [&esp32_ad5933](const BLE_Client::Discovery::Device& e) {
                            return e.get_address() == esp32_ad5933->peripheral.address();
                        }) };
                        if(discovery_devices_update_it != shm->discovery_devices.end()) {
                            *discovery_devices_update_it = tmp_device;
                        }
                    } catch(const SimpleBLE::Exception::BaseException& e) {
                        shm->console.log(std::string("ERROR: BLE_Client::StateMachines::Connection::Actions::disconnect: exception: ") + e.what() + "\n");
                    }
                }
            }

            namespace Guards {
                bool write_body_composition_feature_successful(const BLE_Client::StateMachines::Connection::Events::write_body_composition_feature& event, std::shared_ptr<ESP32_AD5933> esp32_ad5933, std::shared_ptr<BLE_Client::SHM::Parent> shm) {
                    try{
                        std::visit([esp32_ad5933](auto&& event) {
                            esp32_ad5933->write_body_composition_feature(Magic::Commands::Serializer::run(event));
                        }, event.event_variant);
                        return true;
                    } catch(const std::exception& e) {
                        shm->console.log(std::string("ERROR: BLE_Client::StateMachines::Connection::Actions::write_body_composition_feature: exception: ") + e.what() + "\n");
                        return false;
                    }
                }

                bool write_body_composition_feature_failed(const BLE_Client::StateMachines::Connection::Events::write_body_composition_feature& event, std::shared_ptr<ESP32_AD5933> esp32_ad5933, std::shared_ptr<BLE_Client::SHM::Parent> shm) {
                    return !write_body_composition_feature_successful(event, esp32_ad5933, shm);
                }
            }
        }
    }
}