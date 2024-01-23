#pragma once

#include <iostream>
#include <optional>
#include <atomic>
#include <vector>
#include <string>
#include <memory>
#include <string>
#include <queue>
#include <condition_variable>
#include <mutex>

#include "simpleble/SimpleBLE.h"

struct Payload {
    std::queue<std::string> content;
    std::mutex mutex;
    std::condition_variable cv;
};

std::optional<SimpleBLE::Adapter> find_adapter();
std::optional<SimpleBLE::Peripheral> find_esp32_ad5933(const bool &done);
bool is_peripheral_esp32_ad5933(SimpleBLE::Peripheral &peripheral);
class ESP32_AD5933 {
private:
    std::optional<SimpleBLE::Service> body_composistion_service = std::nullopt;
    std::optional<SimpleBLE::Characteristic> body_composition_measurement_chacteristic = std::nullopt;
    std::optional<SimpleBLE::Characteristic> body_composition_feature_chacteristic  = std::nullopt;
    bool is_subscribed_to_body_composition_measurement_chacteristic_indicate = false;
    bool is_subscribed_to_body_composition_measurement_chacteristic_notify = false;
public:
    SimpleBLE::Peripheral peripheral;
    Payload rx_payload;
    std::optional<std::string> temp_payload = std::nullopt;
private:
    bool initialize_connection();
public:
    ESP32_AD5933() = default;
    ESP32_AD5933(SimpleBLE::Peripheral in_peripheral);
    ESP32_AD5933(const ESP32_AD5933 &other);
    ESP32_AD5933& operator=(const ESP32_AD5933& other);
    bool connect();
    void subscribe_to_body_composition_measurement_indicate();
    void unsubscribe_from_body_composistion_measurement();
    void toggle_subscribe_to_body_composition_measurement();
    bool send(const SimpleBLE::ByteArray &data);
    void subscribe_to_body_composition_measurement_notify();
    void disconnect();
    bool is_connected();
    void print_mtu();
    ~ESP32_AD5933();
};

std::ostream& operator<<(std::ostream& os, SimpleBLE::Peripheral& peripheral);
std::ostream& operator<<(std::ostream& os, SimpleBLE::Adapter& adapter);
std::ostream& operator<<(std::ostream& os, SimpleBLE::Service& service);
std::ostream& operator<<(std::ostream& os, SimpleBLE::Characteristic& characteristic);
std::ostream& operator<<(std::ostream& os, SimpleBLE::Descriptor& descriptor);
std::ostream& operator<<(std::ostream& os, std::vector<SimpleBLE::Adapter> &adapters);

