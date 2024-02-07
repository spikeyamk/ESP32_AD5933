#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
#include <string_view>

#include "magic/packets/outcoming.hpp"

#include "ble_client/state_machines/connection/connection.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Connection {
            namespace Actions {
                void disconnect(std::shared_ptr<ESP32_AD5933> esp32_ad5933, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm) {
                    try {
                        if(esp32_ad5933->is_connected() == false) {
                            return;
                        }
                        esp32_ad5933->remove_subscriptions();
                        esp32_ad5933->disconnect();
                    } catch(const SimpleBLE::Exception::BaseException& e) {
                        shm->console.log(std::string("ERROR: BLE_Client::StateMachines::Connection::Actions::disconnect: exception: ") + e.what() + "\n");
                    }
                }
            }

            namespace Guards {
                bool write_body_composition_feature_successful(const BLE_Client::StateMachines::Connection::Events::write_body_composition_feature& event, std::shared_ptr<ESP32_AD5933> esp32_ad5933, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm) {
                    try{
                        std::visit([esp32_ad5933](auto&& event) {
                            using T_Decay = std::decay_t<decltype(event)>;
                            const Magic::OutComingPacket<T_Decay> data_outcoming_packet { event };
                            esp32_ad5933->write_body_composition_feature(data_outcoming_packet.get_raw_data());
                        }, event.event_variant);
                        return true;
                    } catch(const std::exception& e) {
                        shm->console.log(std::string("ERROR: BLE_Client::StateMachines::Connection::Actions::write_event: exception: ") + e.what() + "\n");
                        return false;
                    }
                }

                bool write_body_composition_feature_failed(const BLE_Client::StateMachines::Connection::Events::write_body_composition_feature& event, std::shared_ptr<ESP32_AD5933> esp32_ad5933, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm) {
                    return !write_body_composition_feature_successful(event, esp32_ad5933, shm);
                }
            }
        }
    }
}