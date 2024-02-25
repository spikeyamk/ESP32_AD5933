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
        struct Service : SimpleBLE::Service {
            using Base = SimpleBLE::Service;
            SimpleBLE::Characteristic body_composition_measurement;
            SimpleBLE::Characteristic body_composition_feature;
            SimpleBLE::Characteristic time_update_control_point;
            SimpleBLE::Characteristic hid_information;
            Service() = default;
            Service(
                SimpleBLE::Service& service,
                SimpleBLE::Characteristic body_composition_measurement,
                SimpleBLE::Characteristic body_composition_feature,
                SimpleBLE::Characteristic time_update_control_point,
                SimpleBLE::Characteristic hid_information
            );
        };
    private:
        Service service {};
        struct Channels {
            std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> body_composition_measurement { nullptr };
            std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> hid_information { nullptr };
        };
        Channels channels {};
        std::shared_ptr<BLE_Client::SHM::ChildSHM> child_shm { nullptr };
    public:
        ESP32_AD5933() = default;
        ESP32_AD5933(
            SimpleBLE::Peripheral& peripheral, 
            Service& characteristics,
            std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> body_composition_measurement_channel,
            std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> hid_information_channel,
            std::shared_ptr<BLE_Client::SHM::ChildSHM> child_shm
        );
        void update_time();
        void setup_subscriptions();
        void remove_subscriptions();
    private:
        template<size_t N>
        void write(const std::array<uint8_t, N>& packet, SimpleBLE::Characteristic& characteristic) {
            static_assert(N <= Magic::MTU);
            peripheral.write_request(service.uuid(), characteristic.uuid(), std::string(packet.begin(), packet.end()));

        }
    public:
        template<size_t N>
        void write_time_update_control_point(const std::array<uint8_t, N>& packet) {
            static_assert(N <= Magic::MTU);
            write(packet, service.time_update_control_point);
        }

        template<size_t N>
        void write_body_composition_feature(const std::array<uint8_t, N>& packet) {
            static_assert(N <= Magic::MTU);
            write(packet, service.body_composition_feature);
        }

        bool is_connected();
        void disconnect();
    };

    std::optional<BLE_Client::ESP32_AD5933::Service> find_services_characteristics(SimpleBLE::Peripheral& peripheral);
    std::string get_address_without_semicolons(SimpleBLE::Peripheral& peripheral);
}