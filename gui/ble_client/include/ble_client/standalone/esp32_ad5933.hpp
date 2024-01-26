#pragma once

#include <memory>
#include <simpleble/SimpleBLE.h>
#include <array>
#include <cstdint>
#include <string>

#include "ble_client/standalone/shm.hpp"

namespace BLE_Client {
    class ESP32_AD5933 {
    public:
        SimpleBLE::Peripheral peripheral;
    private:
        SimpleBLE::Service body_composistion_service;
        SimpleBLE::Characteristic body_composition_measurement_chacteristic;
        SimpleBLE::Characteristic body_composition_feature_chacteristic;
        BLE_Client::SHM::SHM* shm = nullptr;
    public:
        ESP32_AD5933() = default;
        ESP32_AD5933(
            SimpleBLE::Peripheral& peripheral, 
            SimpleBLE::Service& body_composistion_service,
            SimpleBLE::Characteristic& body_composition_measurement_chacteristic,
            SimpleBLE::Characteristic& body_composition_feature_chacteristic,
            BLE_Client::SHM::SHM* shm
        );
        void setup_subscriptions();
        void remove_subscriptions();
        void write(const std::array<uint8_t, 20>& packet);
    };
}