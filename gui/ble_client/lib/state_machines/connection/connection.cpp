#include "ble_client/standalone/state_machines/connection/connection.hpp"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
#include <string_view>

namespace BLE_Client {
    namespace StateMachines {
        namespace Connection {
            namespace Actions {
                void disconnect(std::shared_ptr<ESP32_AD5933> esp32_ad5933) {
                    try {
                        if(esp32_ad5933->is_connected() == false) {
                            return;
                        }
                        esp32_ad5933->remove_subscriptions();
                        esp32_ad5933->disconnect();
                    } catch(const SimpleBLE::Exception::BaseException& e) {
                        std::cerr << "ERROR: BLE_Client::StateMachines::Connection::Actions::disconnect: exception: " << e.what() << std::endl;
                    }
                }
            }

            namespace Guards {
                bool write_successful(const BLE_Client::StateMachines::Connection::Events::write& event, std::shared_ptr<ESP32_AD5933> esp32_ad5933) {
                    try{
                        esp32_ad5933->write(event.packet);
                        return true;
                    } catch(const std::exception& e) {
                        std::cerr << "ERROR: BLE_Client::StateMachines::Connection::Actions::write: exception: " << e.what() << std::endl;
                        return false;
                    }
                }
                bool write_failed(const BLE_Client::StateMachines::Connection::Events::write& event, std::shared_ptr<ESP32_AD5933> esp32_ad5933) {
                    return write_successful(event, esp32_ad5933);
                }
            }
        }
    }
}