#include <trielo/trielo.hpp>

#include "magic/commands/serializer.hpp"
#include "magic/results/deserializer.hpp"

#include "ble_client/esp32_ad5933.hpp"

namespace BLE_Client {
    ESP32_AD5933::Service::Service(
        SimpleBLE::Service& service,
        SimpleBLE::Characteristic body_composition_measurement,
        SimpleBLE::Characteristic body_composition_feature,
        SimpleBLE::Characteristic time_update_control_point,
        SimpleBLE::Characteristic hid_information
    ) :
        Base{ service },
        body_composition_measurement { body_composition_measurement },
        body_composition_feature { body_composition_feature },
        time_update_control_point { time_update_control_point },
        hid_information { hid_information }
    {}

    ESP32_AD5933::ESP32_AD5933(
        SimpleBLE::Peripheral& peripheral, 
        Service& characteristics,
        std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> body_composition_measurement_channel,
        std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> hid_information_channel,
        std::shared_ptr<BLE_Client::SHM::ChildSHM> child_shm
    ) :
        peripheral{ peripheral },
        service { characteristics },
        channels{ body_composition_measurement_channel, hid_information_channel },
        child_shm{ child_shm },
        connection_checker{ connection_checker_cb, std::ref(*this) }
    {}

    ESP32_AD5933::~ESP32_AD5933() {
        connection_checker.request_stop();
        connection_checker.join();
    }

    void ESP32_AD5933::connection_checker_cb(std::stop_token st, ESP32_AD5933& self) {
        while(st.stop_requested() == false) {
            auto it { std::find_if(self.child_shm->discovery_devices->begin(), self.child_shm->discovery_devices->end(), [&](const auto& e) {
                return self.peripheral.address() == e.get_address();
            }) };
            if(it != self.child_shm->discovery_devices->end()) {
                it->update_connected(self.peripheral.is_connected());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void ESP32_AD5933::update_time() {
        const auto update_timeval_ser { Magic::Commands::Serializer::run(Magic::Commands::Time::UpdateTimeval::now()) };
        peripheral.write_request(
            service.uuid(),
            service.time_update_control_point.uuid(),
            std::string(update_timeval_ser.begin(), update_timeval_ser.end())
        );
    }

    void ESP32_AD5933::setup_subscriptions() {
        const std::string tmp_address { get_address_without_semicolons(peripheral) };
        peripheral.notify(
            service.uuid(),
            service.body_composition_measurement.uuid(),
            [&](SimpleBLE::ByteArray captured_payload) {
                Magic::T_MaxPacket raw_bytes;
                std::copy(captured_payload.begin(), captured_payload.end(), raw_bytes.begin());
                const auto decoded { Magic::Results::Deserializer::decode(raw_bytes.begin(), raw_bytes.begin() + captured_payload.size()) };
                if(decoded.has_value()) {
                    channels.body_composition_measurement->send(decoded.value());
                }
            }
        );

        peripheral.notify(
            service.uuid(),
            service.hid_information.uuid(),
            [&](SimpleBLE::ByteArray captured_payload) {
                Magic::T_MaxPacket raw_bytes;
                std::copy(captured_payload.begin(), captured_payload.end(), raw_bytes.begin());
                const auto decoded { Magic::Results::Deserializer::decode(raw_bytes.begin(), raw_bytes.begin() + captured_payload.size()) };
                if(decoded.has_value()) {
                    channels.hid_information->send(decoded.value());
                }
            }
        );
    }

    void ESP32_AD5933::remove_subscriptions() {
        peripheral.unsubscribe(
            service.uuid(),
            service.body_composition_measurement.uuid()
        );

        peripheral.unsubscribe(
            service.uuid(),
            service.hid_information.uuid()
        );
    }

    bool ESP32_AD5933::is_connected() {
        return peripheral.is_connected();
    }

    void ESP32_AD5933::disconnect() {
        peripheral.disconnect();
    }

    std::optional<BLE_Client::ESP32_AD5933::Service> find_services_characteristics(SimpleBLE::Peripheral& peripheral) {
        try {
            static constexpr std::string_view BODY_COMPOSITION_SERVICE_UUID { "0000181b-0000-1000-8000-00805f9b34fb" };
            static constexpr std::string_view BODY_COMPOSITION_FEATURE_UUID { "00002a9b-0000-1000-8000-00805f9b34fb" };
            static constexpr std::string_view BODY_COMPOSITION_MEASUREMENT_UUID { "00002a9c-0000-1000-8000-00805f9b34fb" };
            static constexpr std::string_view TIME_UPDATE_CONTROL_POINT_UUID { "00002a16-0000-1000-8000-00805f9b34fb" };
            static constexpr std::string_view HID_INFORMATION_UUID { "00002a4a-0000-1000-8000-00805f9b34fb" };

            static const auto is_characteristic_body_composition_measurement = [&](SimpleBLE::Characteristic &characteristic) {
                if(
                    (characteristic.uuid() == BODY_COMPOSITION_MEASUREMENT_UUID)
                    && characteristic.can_notify()
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
                    && characteristic.can_notify()
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
                //shm->console.log(std::string("ERROR: BLE_Client::ESP32_AD5933::find_services_characteristics: failed to find body_composition_service\n"));
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
                //shm->console.log(std::string("ERROR: BLE_Client::ESP32_AD5933::find_services_characteristics: failed to find body_composition_measurement\n"));
                return std::nullopt;
            }

            const auto it_body_composition_feature { find_it_characteristic(is_characteristic_body_composition_feature) };
            if(it_body_composition_feature.has_value() == false) {
                //shm->console.log(std::string("ERROR: BLE_Client::ESP32_AD5933::find_services_characteristics: failed to find body_composition_feature\n"));
                return std::nullopt;
            }

            const auto it_hid_information { find_it_characteristic(is_hid_information) };
            if(it_hid_information.has_value() == false) {
                //shm->console.log(std::string("ERROR: BLE_Client::ESP32_AD5933::find_services_characteristics: failed to find hid_information\n"));
                return std::nullopt;
            }

            const auto it_time_update_control_point { find_it_characteristic(is_time_update_control_point) };
            if(it_time_update_control_point.has_value() == false) {
                //shm->console.log(std::string("ERROR: BLE_Client::ESP32_AD5933::find_services_characteristics: failed to find time_update_control_point\n"));
                return std::nullopt;
            }

            return BLE_Client::ESP32_AD5933::Service {
                *it_service,
                *it_body_composition_measurement.value(),
                *it_body_composition_feature.value(),
                *it_time_update_control_point.value(),
                *it_hid_information.value()
            };
        } catch(...) {
            //shm->console.log(std::string("ERROR: BLE_Client::ESP32_AD5933::find_services_characteristics: exception: ") + e.what() + "\n");
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