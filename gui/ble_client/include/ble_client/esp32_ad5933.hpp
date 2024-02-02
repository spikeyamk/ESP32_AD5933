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
        SimpleBLE::Characteristic body_composition_measurement_chacteristic;
        SimpleBLE::Characteristic body_composition_feature_chacteristic;
        std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> channel = nullptr;
        std::shared_ptr<BLE_Client::SHM::ChildSHM> child_shm = nullptr;
    public:
        ESP32_AD5933() = default;
        ESP32_AD5933(
            SimpleBLE::Peripheral& peripheral, 
            SimpleBLE::Service& body_composistion_service,
            SimpleBLE::Characteristic& body_composition_measurement_chacteristic,
            SimpleBLE::Characteristic& body_composition_feature_chacteristic,
            std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> channel,
            std::shared_ptr<BLE_Client::SHM::ChildSHM> child_shm
        );
        void setup_subscriptions();
        void remove_subscriptions();

        template<size_t N>
        void write(const std::array<uint8_t, N>& packet) {
            static_assert(N <= Magic::MTU);
            std::printf("BLE_Client::ESP32_AD5933::write<%zu>(): packet: ", N);
            for(size_t i = 0; i < packet.size(); i++) {
                if(i % 8 == 0) {
                    std::printf("\n\t");
                }
                std::printf("0x%02X, ", static_cast<uint8_t>(packet[i]));
            }
            std::printf("\n");
            peripheral.write_request(body_composistion_service.uuid(), body_composition_feature_chacteristic.uuid(), std::string(packet.begin(), packet.end()));
        }

        bool is_connected();
        void disconnect();
    };

    std::optional<std::tuple<SimpleBLE::Service, SimpleBLE::Characteristic, SimpleBLE::Characteristic>> find_services_characteristics(SimpleBLE::Peripheral& peripheral, std::shared_ptr<BLE_Client::SHM::ChildSHM> shm);
    std::string get_address_without_semicolons(SimpleBLE::Peripheral& peripheral);
}