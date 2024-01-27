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

#include "ble_client/standalone/shm.hpp"

namespace BLE_Client {
    class ESP32_AD5933 {
    public:
        SimpleBLE::Peripheral peripheral;
    private:
        SimpleBLE::Service body_composistion_service;
        SimpleBLE::Characteristic body_composition_measurement_chacteristic;
        SimpleBLE::Characteristic body_composition_feature_chacteristic;
        std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> channel = nullptr;
    public:
        ESP32_AD5933() = default;
        ESP32_AD5933(
            SimpleBLE::Peripheral& peripheral, 
            SimpleBLE::Service& body_composistion_service,
            SimpleBLE::Characteristic& body_composition_measurement_chacteristic,
            SimpleBLE::Characteristic& body_composition_feature_chacteristic,
            std::shared_ptr<BLE_Client::SHM::NotifyChannelTX> channel
        );
        void setup_subscriptions();
        void remove_subscriptions();
        void write(const std::array<uint8_t, 20>& packet);
        bool is_connected();
        void disconnect();
    };

    std::optional<std::tuple<SimpleBLE::Service, SimpleBLE::Characteristic, SimpleBLE::Characteristic>> find_services_characteristics(SimpleBLE::Peripheral& peripheral);
    std::string get_address_without_semicolons(SimpleBLE::Peripheral& peripheral);
}