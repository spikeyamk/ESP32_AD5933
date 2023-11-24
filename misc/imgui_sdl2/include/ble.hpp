#pragma once

#include <optional>
#include <atomic>
#include <string>
#include <memory>
#include <string>

#include "simpleble/SimpleBLE.h"

namespace BLE_Payloads {
    //extern std::atomic<std::shared_ptr<std::string>> rx_payload;
}

class Payload {
public:
    std::atomic<bool> read_ready { false };
    std::string content;
    Payload() = default;
    Payload(const Payload &other);
    Payload& operator=(const Payload &&other);
    std::string read();
    void clean();
};

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
    Payload rx_payload;
    std::optional<std::string> temp_payload = std::nullopt;
private:
    bool initialize_connection();
public:
    ESP32_AD5933(SimpleBLE::Peripheral in_peripheral);
    ESP32_AD5933(const ESP32_AD5933 &other);
    ESP32_AD5933& operator=(const ESP32_AD5933& other);
    bool connect();
    void subscribe_to_body_composition_measurement_indicate();
    void unsubscribe_from_body_composistion_measurement();
    void toggle_subscribe_to_body_composition_measurement();
    void send(const SimpleBLE::ByteArray &data);
    void subscribe_to_body_composition_measurement_notify();
    void disconnect();
    bool is_connected();
    void print_mtu();
    ~ESP32_AD5933();
};

extern std::optional<ESP32_AD5933> esp32_ad5933;
