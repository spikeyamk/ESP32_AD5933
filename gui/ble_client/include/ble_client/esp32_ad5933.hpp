#pragma once

#include <memory>
#include <simpleble/SimpleBLE.h>
#include <array>
#include <cstdint>
#include <string>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <algorithm>
#include <optional>
#include <tuple>

#include "ble_client/shm/child/child.hpp"
#include "magic/events/common.hpp"

namespace BLE_Client {
    class ESP32_AD5933 {
    public:
        SimpleBLE::Peripheral peripheral;
    private:
        SimpleBLE::Service body_composistion_service;
        struct Characteristics {
            SimpleBLE::Characteristic body_composition_measurement;
            SimpleBLE::Characteristic body_composition_feature;
            SimpleBLE::Characteristic time_update_control_point;
            SimpleBLE::Characteristic hid_information;
        };
        Characteristics characteristics {};
        struct Channels {
            std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> body_composition_measurement = nullptr;
            std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> hid_information = nullptr;
        };
        Channels channels {};
        std::shared_ptr<BLE_Client::SHM::ChildSHM> child_shm = nullptr;
    public:
        ESP32_AD5933() = default;
        ESP32_AD5933(
            SimpleBLE::Peripheral& peripheral, 
            SimpleBLE::Service& body_composistion_service,
            SimpleBLE::Characteristic& body_composition_measurement_characteristic,
            SimpleBLE::Characteristic& body_composition_feature_characteristic,
            SimpleBLE::Characteristic& time_update_control_point_characteristic,
            SimpleBLE::Characteristic& hid_information_characteristic,
            std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> body_composition_measurement_channel,
            std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> hid_information_channel,
            std::shared_ptr<BLE_Client::SHM::ChildSHM> child_shm
        );
        void setup_subscriptions_and_update_time();
        void remove_subscriptions();
    private:
        template<size_t N>
        void write(const std::array<uint8_t, N>& packet, SimpleBLE::Characteristic& characteristic) {
            static_assert(N <= Magic::MTU);
            peripheral.write_request(body_composistion_service.uuid(), characteristic.uuid(), std::string(packet.begin(), packet.end()));
        }
    public:
        template<size_t N>
        void write_time_update_control_point(const std::array<uint8_t, N>& packet) {
            static_assert(N <= Magic::MTU);
            write(packet, characteristics.time_update_control_point);
        }

        template<size_t N>
        void write_body_composition_feature(const std::array<uint8_t, N>& packet) {
            static_assert(N <= Magic::MTU);
            write(packet, characteristics.body_composition_feature);
        }

        bool is_connected();
        void disconnect();
    };

    std::optional<std::tuple<SimpleBLE::Service, SimpleBLE::Characteristic, SimpleBLE::Characteristic, SimpleBLE::Characteristic, SimpleBLE::Characteristic>> find_services_characteristics(SimpleBLE::Peripheral& peripheral);
    std::string get_address_without_semicolons(SimpleBLE::Peripheral& peripheral);
}