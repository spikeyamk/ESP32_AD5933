#include "ble_client/standalone/esp32_ad5933.hpp"
#include <trielo/trielo.hpp>

namespace BLE_Client {
    ESP32_AD5933::ESP32_AD5933(
        SimpleBLE::Peripheral& peripheral, 
        SimpleBLE::Service& body_composistion_service,
        SimpleBLE::Characteristic& body_composition_measurement_chacteristic,
        SimpleBLE::Characteristic& body_composition_feature_chacteristic,
        std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> channel
    ) :
        peripheral{ peripheral },
        body_composistion_service{ body_composistion_service },
        body_composition_measurement_chacteristic{ body_composition_measurement_chacteristic },
        body_composition_feature_chacteristic{ body_composition_feature_chacteristic },
        channel{ channel }
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
                channel->send(raw_bytes);
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

    bool ESP32_AD5933::is_connected() {
        try {
            return peripheral.is_connected();
        } catch(...) {
            return false;
        }
    }

    void ESP32_AD5933::disconnect() {
        peripheral.disconnect();
    }

    std::optional<std::tuple<SimpleBLE::Service, SimpleBLE::Characteristic, SimpleBLE::Characteristic>> find_services_characteristics(SimpleBLE::Peripheral& peripheral) {
        try {
            static constexpr std::string_view BODY_COMPOSITION_SERVICE_UUID { "0000181b-0000-1000-8000-00805f9b34fb" };
            static constexpr std::string_view BODY_COMPOSITION_FEATURE_UUID { "00002a9b-0000-1000-8000-00805f9b34fb" };
            static constexpr std::string_view BODY_COMPOSITION_MEASUREMENT_UUID { "00002a9c-0000-1000-8000-00805f9b34fb" };

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
                return std::nullopt;
            }

            auto characteristics { it_service->characteristics() };
            auto it_body_composition_measurement = std::find_if(characteristics.begin(), characteristics.end(), [&](SimpleBLE::Characteristic& e) {
                return is_characteristic_body_composition_measurement(e);
            });
            if(it_body_composition_measurement == characteristics.end()) {
                return std::nullopt;
            }

            auto it_body_composition_feature = std::find_if(characteristics.begin(), characteristics.end(), [&](SimpleBLE::Characteristic& e) {
                return is_characteristic_body_composition_feature(e);
            });
            if(it_body_composition_feature == characteristics.end()) {
                return std::nullopt;
            }

            return std::optional{ std::make_tuple(*it_service, *it_body_composition_measurement, *it_body_composition_feature) };
        } catch(const std::exception& e) {
            std::cerr << "ERROR: BLE_Client::StateMachines::Connection::Guards::is_esp32_ad5933: exception: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::string get_address_without_semicolons(SimpleBLE::Peripheral& peripheral) {
        auto ret { peripheral.address() };
        for(auto& e: ret) {
            if(e == ':') {
                e = '.';
            }
        }
        return ret;
    }
}