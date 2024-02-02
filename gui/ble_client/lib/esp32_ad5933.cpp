#include <trielo/trielo.hpp>

#include "magic/packets/incoming.hpp"
#include "magic/events/results.hpp"
#include "ble_client/esp32_ad5933.hpp"

namespace BLE_Client {
    ESP32_AD5933::ESP32_AD5933(
        SimpleBLE::Peripheral& peripheral, 
        SimpleBLE::Service& body_composistion_service,
        SimpleBLE::Characteristic& body_composition_measurement_chacteristic,
        SimpleBLE::Characteristic& body_composition_feature_chacteristic,
        std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> channel,
        std::shared_ptr<BLE_Client::SHM::ChildSHM> child_shm
    ) :
        peripheral{ peripheral },
        body_composistion_service{ body_composistion_service },
        body_composition_measurement_chacteristic{ body_composition_measurement_chacteristic },
        body_composition_feature_chacteristic{ body_composition_feature_chacteristic },
        channel{ channel },
        child_shm{ child_shm }
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
                child_shm->console.log("BLE_Client::SimpleBLE::Peripheral::notify_callback\n");
                std::printf("BLE_Client::SimpleBLE::Peripheral::notify_callback: captured_payload: ");
                for(size_t i = 0; i < captured_payload.size(); i++) {
                    if(i % 8 == 0) { 
                        std::printf("\n\t");
                    }
                    std::printf("0x%02X, ", static_cast<uint8_t>(captured_payload[i]));
                }
                std::printf("\n");
                Magic::T_MaxPacket raw_bytes;
                std::copy(captured_payload.begin(), captured_payload.end(), raw_bytes.begin());
                const Magic::InComingPacket<Magic::Events::Results::Variant, Magic::Events::Results::Map> incoming_packet { raw_bytes };
                const auto result_event_variant_opt { incoming_packet.to_event_variant() };
                if(result_event_variant_opt.has_value()) {
                    channel->send(result_event_variant_opt.value());
                }
            }
        );
    }

    void ESP32_AD5933::remove_subscriptions() {
        peripheral.unsubscribe(
            body_composistion_service.uuid(),
            body_composition_measurement_chacteristic.uuid()
        );
    }

    bool ESP32_AD5933::is_connected() {
        return peripheral.is_connected();
    }

    void ESP32_AD5933::disconnect() {
        peripheral.disconnect();
    }

    std::optional<std::tuple<SimpleBLE::Service, SimpleBLE::Characteristic, SimpleBLE::Characteristic>> find_services_characteristics(SimpleBLE::Peripheral& peripheral, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm) {
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
            shm->console.log(std::string("ERROR: BLE_Client::StateMachines::Connection::Guards::is_esp32_ad5933: exception: ") + e.what() + "\n");
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