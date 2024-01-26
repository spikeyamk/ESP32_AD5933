#include "ble_client/standalone/esp32_ad5933.hpp"
#include <trielo/trielo.hpp>

namespace BLE_Client {
    ESP32_AD5933::ESP32_AD5933(
        SimpleBLE::Peripheral& peripheral, 
        SimpleBLE::Service& body_composistion_service,
        SimpleBLE::Characteristic& body_composition_measurement_chacteristic,
        SimpleBLE::Characteristic& body_composition_feature_chacteristic,
        std::shared_ptr<BLE_Client::SHM::SHM> shm
    ) :
        peripheral{ peripheral },
        body_composistion_service{ body_composistion_service },
        body_composition_measurement_chacteristic{ body_composition_measurement_chacteristic },
        body_composition_feature_chacteristic{ body_composition_feature_chacteristic },
        shm{ shm }
    {}

    void ESP32_AD5933::setup_subscriptions() {
        std::string tmp_address { peripheral.address() };
        for(auto &e: tmp_address) {
            if(e == ':') {
                e = '.';
            }
        }

        static const std::string prefix { "BLE_Client.SHM.connection." + tmp_address + "." };
        const std::string notify_deque_name { prefix + "notify_deque" };
        const std::string notify_deque_mutex_name { prefix + "notify_mutex" };
        const std::string notify_deque_condition_name { prefix + "notify_condition" };

        peripheral.notify(
            body_composistion_service.uuid(),
            body_composition_measurement_chacteristic.uuid(),
            [&](SimpleBLE::ByteArray captured_payload) {
                fmt::print(fmt::fg(fmt::color::red), "BLE_Client::SimpleBLE::Peripheral::notify_callback: we got a notify packet\n");
                std::array<uint8_t, 20> raw_bytes;
                std::copy(captured_payload.begin(), captured_payload.end(), raw_bytes.begin());
                shm->send_notify(raw_bytes);
            }
        );
    }

    void ESP32_AD5933::remove_subscriptions() {
        peripheral.unsubscribe(
            body_composistion_service.uuid(),
            body_composition_measurement_chacteristic.uuid()
        );
    }

    void ESP32_AD5933::write(const std::array<uint8_t, 20>& packet) {
        peripheral.write_request(body_composistion_service.uuid(), body_composition_feature_chacteristic.uuid(), std::string(packet.begin(), packet.end()));
    }
}