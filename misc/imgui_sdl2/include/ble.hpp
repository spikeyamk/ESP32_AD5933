#pragma once

#include <optional>
#include <string>

#include "simpleble/SimpleBLE.h"

std::optional<SimpleBLE::Peripheral> find_esp32_ad5933();
bool is_peripheral_esp32_ad5933(SimpleBLE::Peripheral &peripheral);
class ESP32_AD5933 {
private:
    SimpleBLE::Peripheral peripheral;
    std::optional<SimpleBLE::Service> body_composistion_service = std::nullopt;
    std::optional<SimpleBLE::Characteristic> body_composition_measurement_chacteristic = std::nullopt;
    std::optional<SimpleBLE::Characteristic> body_composition_feature_chacteristic  = std::nullopt;
    bool is_subscribed_to_body_composition_measurement_chacteristic_indicate = false;
    bool is_subscribed_to_body_composition_measurement_chacteristic_notify = false;
public:
    std::optional<std::string> temp_payload = std::nullopt;
    std::optional<std::string> rx_payload = std::nullopt;
private:
    bool initialize_connection();
public:
    ESP32_AD5933(SimpleBLE::Peripheral in_peripheral);
    bool connect();
    void subscribe_to_body_composition_measurement_indicate();
    void unsubscribe_from_body_composistion_measurement();
    void toggle_subscribe_to_body_composition_measurement();
    void send(const SimpleBLE::ByteArray &data);
    void subscribe_to_body_composition_measurement_notify();
    void disconnect();
    ~ESP32_AD5933();
};

extern std::optional<ESP32_AD5933> esp32_ad5933;
