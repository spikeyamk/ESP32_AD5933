#include <trielo/trielo.hpp>

#include "magic/packets/incoming.hpp"
#include "magic/events/results.hpp"
#include "ble_client/standalone/esp32_ad5933.hpp"

namespace BLE_Client {
    ESP32_AD5933::ESP32_AD5933(
        SimpleBLE::Peripheral& peripheral, 
        SimpleBLE::Service& body_composistion_service,
        SimpleBLE::Characteristic& body_composition_measurement_characteristic,
        SimpleBLE::Characteristic& body_composition_feature_characteristic,
        SimpleBLE::Characteristic& time_update_control_point_characteristic,
        SimpleBLE::Characteristic& hid_information_characteristic,
        std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> body_composition_measurement_channel,
        std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> hid_information_channel,
        std::shared_ptr<BLE_Client::SHM::ChildSHM> child_shm
    ) :
        peripheral{ peripheral },
        body_composistion_service{ body_composistion_service },
        characteristics {
            body_composition_measurement_characteristic,
            body_composition_feature_characteristic,
            time_update_control_point_characteristic,
            hid_information_characteristic
        },
        channels{ body_composition_measurement_channel, hid_information_channel },
        child_shm{ child_shm }
    {}

    void ESP32_AD5933::setup_subscriptions_and_update_time() {
        const std::string tmp_address { get_address_without_semicolons(peripheral) };
        peripheral.indicate(
            body_composistion_service.uuid(),
            characteristics.body_composition_measurement.uuid(),
            [&](SimpleBLE::ByteArray captured_payload) {
                child_shm->console.log("BLE_Client::SimpleBLE::Peripheral::body_composition_measurement::notify_callback\n");
                Magic::T_MaxPacket raw_bytes;
                std::copy(captured_payload.begin(), captured_payload.end(), raw_bytes.begin());
                const Magic::InComingPacket<Magic::Events::Results::Variant, Magic::Events::Results::Map> incoming_packet { raw_bytes };
                const auto result_event_variant_opt { incoming_packet.to_event_variant() };
                if(result_event_variant_opt.has_value()) {
                    channels.body_composition_measurement->send(result_event_variant_opt.value());
                }
            }
        );

        peripheral.indicate(
            body_composistion_service.uuid(),
            characteristics.body_composition_measurement.uuid(),
            [&](SimpleBLE::ByteArray captured_payload) {
                child_shm->console.log("BLE_Client::SimpleBLE::Peripheral::hid_information::notify_callback\n");
                Magic::T_MaxPacket raw_bytes;
                std::copy(captured_payload.begin(), captured_payload.end(), raw_bytes.begin());
                const Magic::InComingPacket<Magic::Events::Results::Variant, Magic::Events::Results::Map> incoming_packet { raw_bytes };
                const auto result_event_variant_opt { incoming_packet.to_event_variant() };
                if(result_event_variant_opt.has_value()) {
                    channels.hid_information->send(result_event_variant_opt.value());
                }
            }
        );

        Magic::Events::Commands::Time::UpdateTimeval timeval_command;
        Magic::Events::Commands::Time::UpdateTimezone timezone_command;
        gettimeofday(&timeval_command.tv, &timezone_command.tz);
        const auto timeval_command_serialized { timeval_command.to_raw_data() };
        peripheral.write_command(
            body_composistion_service.uuid(),
            characteristics.time_update_control_point.uuid(),
            std::string(timeval_command_serialized.begin(), timeval_command_serialized.end())
        );
        const auto timezone_command_serialized { timezone_command.to_raw_data() };
        peripheral.write_command(
            body_composistion_service.uuid(),
            characteristics.time_update_control_point.uuid(),
            std::string(timezone_command_serialized.begin(), timezone_command_serialized.end())
        );
    }

    void ESP32_AD5933::remove_subscriptions() {
        peripheral.unsubscribe(
            body_composistion_service.uuid(),
            characteristics.body_composition_measurement.uuid()
        );

        peripheral.unsubscribe(
            body_composistion_service.uuid(),
            characteristics.hid_information.uuid()
        );
    }

    bool ESP32_AD5933::is_connected() {
        return peripheral.is_connected();
    }

    void ESP32_AD5933::disconnect() {
        peripheral.disconnect();
    }

    std::optional<std::tuple<SimpleBLE::Service, SimpleBLE::Characteristic, SimpleBLE::Characteristic, SimpleBLE::Characteristic, SimpleBLE::Characteristic>> find_services_characteristics(SimpleBLE::Peripheral& peripheral, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm) {
        try {
            static constexpr std::string_view BODY_COMPOSITION_SERVICE_UUID { "0000181b-0000-1000-8000-00805f9b34fb" };
            static constexpr std::string_view BODY_COMPOSITION_FEATURE_UUID { "00002a9b-0000-1000-8000-00805f9b34fb" };
            static constexpr std::string_view BODY_COMPOSITION_MEASUREMENT_UUID { "00002a9c-0000-1000-8000-00805f9b34fb" };
            static constexpr std::string_view TIME_UPDATE_CONTROL_POINT_UUID { "00002a16-0000-1000-8000-00805f9b34fb" };
            static constexpr std::string_view HID_INFORMATION_UUID { "00002a4a-0000-1000-8000-00805f9b34fb" };

            static const auto is_characteristic_body_composition_measurement = [&](SimpleBLE::Characteristic &characteristic) {
                if(
                    (characteristic.uuid() == BODY_COMPOSITION_MEASUREMENT_UUID)
                    && characteristic.can_indicate()
                ) {
                    return true;
                } else {
                    return false;
                }
            };

            static const auto is_characteristic_body_composition_feature = [&](SimpleBLE::Characteristic &characteristic) {
                if(
                    (characteristic.uuid() == BODY_COMPOSITION_FEATURE_UUID)
                    && characteristic.can_write_request()
                ) {
                    return true;
                } else {
                    return false;
                }
            };

            static const auto is_time_update_control_point = [&](SimpleBLE::Characteristic &characteristic) {
                if(
                    (characteristic.uuid() == TIME_UPDATE_CONTROL_POINT_UUID)
                    && characteristic.can_write_request()
                ) {
                    return true;
                } else {
                    return false;
                }
            };

            static const auto is_hid_information = [&](SimpleBLE::Characteristic &characteristic) {
                if(
                    (characteristic.uuid() == HID_INFORMATION_UUID)
                    && characteristic.can_indicate()
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
                shm->console.log(std::string("ERROR: BLE_Client::ESP32_AD5933::find_services_characteristics: failed to find body_composition_service\n"));
                return std::nullopt;
            }

            auto characteristics { it_service->characteristics() };

            static const auto find_it_characteristic = [&](auto predicate) -> std::optional<std::vector<SimpleBLE::Characteristic>::iterator> {
                const auto find_it = std::find_if(characteristics.begin(), characteristics.end(), [&](SimpleBLE::Characteristic& e) {
                    return predicate(e);
                });
                if(find_it == characteristics.end()) {
                    return std::nullopt;
                }
                return find_it;
            };

            const auto it_body_composition_measurement { find_it_characteristic(is_characteristic_body_composition_measurement) };
            if(it_body_composition_measurement.has_value() == false) {
                shm->console.log(std::string("ERROR: BLE_Client::ESP32_AD5933::find_services_characteristics: failed to find body_composition_measurement\n"));
                return std::nullopt;
            }

            const auto it_body_composition_feature { find_it_characteristic(is_characteristic_body_composition_feature) };
            if(it_body_composition_feature.has_value() == false) {
                shm->console.log(std::string("ERROR: BLE_Client::ESP32_AD5933::find_services_characteristics: failed to find body_composition_feature\n"));
                return std::nullopt;
            }

            const auto it_hid_information { find_it_characteristic(is_hid_information) };
            if(it_hid_information.has_value() == false) {
                shm->console.log(std::string("ERROR: BLE_Client::ESP32_AD5933::find_services_characteristics: failed to find hid_information\n"));
                return std::nullopt;
            }

            const auto it_time_update_control_point { find_it_characteristic(is_time_update_control_point) };
            if(it_time_update_control_point.has_value() == false) {
                shm->console.log(std::string("ERROR: BLE_Client::ESP32_AD5933::find_services_characteristics: failed to find time_update_control_point\n"));
                return std::nullopt;
            }

            return std::optional{ std::make_tuple(*it_service, *it_body_composition_measurement.value(), *it_body_composition_feature.value(), *it_hid_information.value(), *it_time_update_control_point.value()) };
        } catch(const std::exception& e) {
            shm->console.log(std::string("ERROR: BLE_Client::ESP32_AD5933::find_services_characteristics: exception: ") + e.what() + "\n");
            return std::nullopt;
        }
    }

    std::string get_address_without_semicolons(SimpleBLE::Peripheral& peripheral) {
        std::string ret { peripheral.address() };
        for(auto& e: ret) {
            if(e == ':') {
                e = '.';
            }
        }
        return ret;
    }
}