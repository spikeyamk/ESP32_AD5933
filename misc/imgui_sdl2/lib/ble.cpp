#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <optional>
#include <utility>
#include <atomic>
#include <memory>

#include <simpleble/SimpleBLE.h>
#include "trielo.hpp"
#include "fmt/core.h"
#include "fmt/color.h"

#include "ble.hpp"

const std::string BLE_ESP_IDENTIFIER { "nimble" };
const std::string BLE_ESP_ADDRESS { "40:4c:ca:43:11:b2" };

const std::string BODY_COMPOSITION_SERVICE_UUID { "0000181b-0000-1000-8000-00805f9b34fb" };
const std::string BODY_COMPOSITION_FEATURE_UUID { "00002a9b-0000-1000-8000-00805f9b34fb" };
const std::string BODY_COMPOSITION_MEASUREMENT_UUID { "00002a9c-0000-1000-8000-00805f9b34fb" };

std::ostream& operator<<(std::ostream& os, SimpleBLE::Peripheral& peripheral) {
    os << "SimpleBLE: Peripheral Information:" << std::endl;
    os << "\tInitialized: " << peripheral.initialized() << std::endl;
    os << "\tIdentifier: " << peripheral.identifier() << std::endl;
    os << "\tAddress: " << peripheral.address() << std::endl;
    os << "\tAddress Type: " << peripheral.address_type() << std::endl;
    os << "\tRSSI: " << peripheral.rssi() << " dBm" << std::endl;
    os << "\tTX Power: " << peripheral.tx_power() << " dBm" << std::endl;
    os << "\tMTU: " << peripheral.mtu() << std::endl;
    os << "\tConnected: " << peripheral.is_connected() << std::endl;
    os << "\tConnectable: " << peripheral.is_connectable() << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, SimpleBLE::Adapter& adapter) {
    os << "SimpleBLE: Adapter Information:" << std::endl;
    os << "\tAdapter Address: " << adapter.address();
    os << "\tAdapter Identifier: " << adapter.identifier();
    return os;
}

std::ostream& operator<<(std::ostream& os, SimpleBLE::Service& service) {
    os << "SimpleBLE: Service Information:" << std::endl;
    os << "\tService UUID: " << service.uuid() << std::endl;
    os << "\tService data: " << service.data() << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, SimpleBLE::Characteristic& characteristic) {
    os << "SimpleBLE: Characteristic Information:" << std::endl;
    os << "\tCharacteristic UUID: " << characteristic.uuid() << std::endl;
    os << "\tCharacteristic can_read: " << characteristic.can_read() << std::endl;
    os << "\tCharacteristic can_write_request: " << characteristic.can_write_request() << std::endl;
    os << "\tCharacteristic can_write_command: " << characteristic.can_write_command() << std::endl;
    os << "\tCharacteristic can_notify: " << characteristic.can_notify() << std::endl;
    os << "\tCharacteristic can_indicate: " << characteristic.can_indicate() << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, SimpleBLE::Descriptor& descriptor) {
    os << "SimpleBLE: Descriptor Information: Descriptor UUID: " << descriptor.uuid();
    return os;
}

std::ostream& operator<<(std::ostream& os, std::vector<SimpleBLE::Adapter> &adapters) {
    for(auto &i: adapters) {
        os << i << std::endl;
    }
    return os;
}

Payload::Payload(const Payload &other) :
    read_ready { other.read_ready.load() },
    content { other.content }
{}

Payload& Payload::operator=(const Payload &&other) {
    return *this;
}

void Payload::clean() {
    content = std::string {};
    read_ready.store(false);
}

std::string Payload::read() {
    read_ready.wait(false);
    return content;
}

bool is_peripheral_esp32_ad5933(SimpleBLE::Peripheral &peripheral) {
    if(peripheral.address() == BLE_ESP_ADDRESS && peripheral.identifier() == BLE_ESP_IDENTIFIER) {
        return true;
    } else {
        return false;
    }
}

bool is_characteristic_body_composition_measurement(SimpleBLE::Characteristic &characteristic) {
    if(characteristic.uuid() == BODY_COMPOSITION_MEASUREMENT_UUID &&
      (characteristic.can_indicate() && characteristic.can_notify()))
    {
        return true;
    } else {
        return false;
    }
}

bool is_characteristic_body_composition_feature(SimpleBLE::Characteristic &characteristic) {
    if(characteristic.uuid() == BODY_COMPOSITION_FEATURE_UUID &&
      (characteristic.can_write_request()) 
    ) {
        return true;
    } else {
        return false;
    }
}

bool is_service_body_composition(SimpleBLE::Service &service) {
    if(service.uuid() == BODY_COMPOSITION_SERVICE_UUID) {
        return true;
    } else {
        return false;
    }
}

bool ESP32_AD5933::initialize_connection() {
    std::cout << "ESP32_AD5933: Trying to initialize connection\n";
    if(peripheral.is_connectable() == false) {
        fmt::print(fmt::fg(fmt::color::red), "ERROR: BLE: ESP32_AD5933: is not connectable\n");
        return false;
    }

    if(peripheral.is_connected() == true) {
        fmt::print(fmt::fg(fmt::color::red), "ERROR: BLE: ESP32_AD5933: is already connected\n");
        return false;
    }

    peripheral.connect();
    while(peripheral.is_connected() == false) {
        std::cout << "BLE: ESP32_AD5933: Connecting...\n";
    }

    if(peripheral.is_connected()) {
        std::cout << "BLE: Succesfully connected to ESP32_AD5933" << std::endl;
        std::cout << peripheral << std::endl;
        return true;
    }
    return false;
}

ESP32_AD5933::ESP32_AD5933(SimpleBLE::Peripheral in_peripheral) :
    peripheral{in_peripheral}
{}

ESP32_AD5933::ESP32_AD5933(const ESP32_AD5933 &other) :
    peripheral { other.peripheral },
    body_composistion_service { other.body_composistion_service },
    body_composition_measurement_chacteristic { other.body_composition_measurement_chacteristic },
    body_composition_feature_chacteristic { other.body_composition_feature_chacteristic },
    is_subscribed_to_body_composition_measurement_chacteristic_indicate { other.is_subscribed_to_body_composition_measurement_chacteristic_indicate },
    is_subscribed_to_body_composition_measurement_chacteristic_notify { other.is_subscribed_to_body_composition_measurement_chacteristic_notify },
    rx_payload { other.rx_payload }
{}

ESP32_AD5933& ESP32_AD5933::operator=(const ESP32_AD5933& other) {
    return *this;
}

bool ESP32_AD5933::connect() {
    std::cout << "ESP32_AD5933: connect() method called\n";
    if(initialize_connection() == false) {
        fmt::print(fmt::fg(fmt::color::red), "ERROR: BLE: ESP32_AD5933: could not initialize connection\n");
        return false;
    }
    
    for(auto &service: peripheral.services()) {
        if(is_service_body_composition(service)) {
            std::cout << "BLE: ESP32_AD5933: Found Body composition service:\n";
            std::cout << service << std::endl;
            body_composistion_service = service;
        }
    }

    if(body_composistion_service.has_value() == false) {
        fmt::print(fmt::fg(fmt::color::red), "ERROR: BLE: ESP32_AD5933: Body composition service not found\n");
        return false;
    }

    for(auto &characteristic: body_composistion_service.value().characteristics()) {
       if(is_characteristic_body_composition_measurement(characteristic)) {
            std::cout << "BLE: ESP32_AD5933: Found Body Composition Measurement Characteristic:" << std::endl;
            std::cout << characteristic << std::endl;
            body_composition_measurement_chacteristic = characteristic;
        } else if(is_characteristic_body_composition_feature(characteristic)) {
            std::cout << "BLE: ESP32_AD5933: Found Body Composition Feature Characteristic:" << std::endl;
            std::cout << characteristic << std::endl;
            body_composition_feature_chacteristic = characteristic;
        }
    }

    if(body_composition_measurement_chacteristic.has_value() == false) {
        fmt::print(fmt::fg(fmt::color::red), "ERROR: BLE: ESP32_AD5933: Body composition measurement characteristic not found\n");
        return false;
    }

    if(body_composition_feature_chacteristic.has_value() == false) {
        fmt::print(fmt::fg(fmt::color::red), "ERROR: BLE: ESP32_AD5933: Body composition feature characteristic not found\n");
        return false;
    }

    return true;
}

void ESP32_AD5933::subscribe_to_body_composition_measurement_indicate() { 
    if(body_composistion_service.has_value() &&
    body_composition_measurement_chacteristic.has_value() &&
    is_subscribed_to_body_composition_measurement_chacteristic_indicate == false) {
        std::cout << "ESP32_AD5933: Subscribing: Body composition measurement...\n";
        peripheral.indicate(
            body_composistion_service.value().uuid(),
            body_composition_measurement_chacteristic.value().uuid(),
            [&](SimpleBLE::ByteArray payload) {
                std::cout << "Body composition service subscribe event indicate: " << payload << std::endl;
                temp_payload = payload;
            }
        );
        is_subscribed_to_body_composition_measurement_chacteristic_indicate = true;
    }
}

void ESP32_AD5933::unsubscribe_from_body_composistion_measurement() { 
    if(body_composistion_service.has_value() &&
    body_composition_measurement_chacteristic.has_value() &&
    is_subscribed_to_body_composition_measurement_chacteristic_indicate == true) {
        std::cout << "ESP32_AD5933: Unsubscribing: Body composition measurement...\n";
        peripheral.unsubscribe(
            body_composistion_service.value().uuid(),
            body_composition_measurement_chacteristic.value().uuid()
        );
        is_subscribed_to_body_composition_measurement_chacteristic_indicate = false;
    }
    temp_payload = std::nullopt;
}

void ESP32_AD5933::toggle_subscribe_to_body_composition_measurement() {
    if(is_subscribed_to_body_composition_measurement_chacteristic_indicate) {
        unsubscribe_from_body_composistion_measurement();
    } else {
        subscribe_to_body_composition_measurement_indicate();
    }
}

void ESP32_AD5933::disconnect() {
    if(peripheral.is_connected()) {
        std::cout << "ESP32_AD5933: Disconnecting...\n";
        unsubscribe_from_body_composistion_measurement();
        peripheral.disconnect();
    }
}

void ESP32_AD5933::send(const SimpleBLE::ByteArray &data) {
    if(peripheral.is_connected() && body_composistion_service.has_value() && body_composition_feature_chacteristic.has_value()) {
        peripheral.write_request(body_composistion_service.value().uuid(), body_composition_feature_chacteristic.value().uuid(), data);
    }
}

void ESP32_AD5933::subscribe_to_body_composition_measurement_notify() {
    if(peripheral.is_connected() && body_composistion_service.has_value() && body_composition_measurement_chacteristic.has_value()) {
        peripheral.notify(
            body_composistion_service.value().uuid(),
            body_composition_measurement_chacteristic.value().uuid(),
            [&](SimpleBLE::ByteArray captured_payload) {
                // Only for debugging
                /*
                std::cout << "Printing from subscribe_to_body_composition_measurement_notify lambda\n";
                for(size_t i = 0; i < payload.size(); i++) {
                    std::printf("Byte[%zu]: 0x%02X\n", static_cast<uint8_t>(payload[i]));
                }
                */
                while(rx_payload.read_ready.load() == true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                rx_payload.content = captured_payload;
                rx_payload.read_ready.store(true);
                rx_payload.read_ready.notify_all();
            }
        );
    }
}

ESP32_AD5933::~ESP32_AD5933() {
    disconnect();
}

std::optional<SimpleBLE::Peripheral> find_esp32_ad5933() {
    if(Trielo::trielo<SimpleBLE::Adapter::bluetooth_enabled>(Trielo::OkErrCode(true)) == false) {
        std::cout << "ERROR: BLE: Bleutooth disabled\n";
        return std::nullopt;
    }

    auto adapters = Trielo::trielo<SimpleBLE::Adapter::get_adapters>();
    if(adapters.empty()) {
        std::cout << "ERROR: BLE: Could not find a Bluetooth adapter\n";
        return std::nullopt;
    }

    // Use the first adapter
    auto adapter = adapters[0];

    // Do something with the adapter
    std::cout << "BLE: Using the default adapter\n ";

    if(adapter.bluetooth_enabled() == false) {
        fmt::print(fmt::fg(fmt::color::yellow), "WARNING: BLE: Default adapter: Bleutooth disabled\n");
        return std::nullopt;
    }

    adapter.set_callback_on_scan_start([]() { std::cout << "BLE: Scan started." << std::endl; });
    adapter.set_callback_on_scan_stop([]() { std::cout << "BLE: Scan stopped." << std::endl; });

    std::optional<SimpleBLE::Peripheral> esp32_ad5933_peripheral = std::nullopt;
    adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral peripheral) {
        if(is_peripheral_esp32_ad5933(peripheral)) {
            std::cout << "BLE: Found ESP32 AD5933\n";
            std::cout << peripheral;
            esp32_ad5933_peripheral = peripheral;
            if(adapter.scan_is_active()) {
                adapter.scan_stop();
            }
        }
    });

    adapter.set_callback_on_scan_updated([&](SimpleBLE::Peripheral peripheral) {
        if(is_peripheral_esp32_ad5933(peripheral)) {
            std::cout << "BLE: ESP32 AD5933: Updated status\n";
            std::cout << peripheral;
            esp32_ad5933_peripheral = peripheral;
            if(adapter.scan_is_active()) {
                adapter.scan_stop();
            }
        }
    });
    
    adapter.scan_for(10'000);

    if(esp32_ad5933_peripheral.has_value() == false) {
        fmt::print(fmt::fg(fmt::color::red), "ERROR: BLE: Default adapter: Failed to find ESP32 AD5933\n");
        return std::nullopt;
    }
    
    return std::optional { esp32_ad5933_peripheral };
}

bool ESP32_AD5933::is_connected() {
    return peripheral.is_connected();
}

void ESP32_AD5933::print_mtu() {
    std::cout << "ESP32_AD5933: MTU: " << peripheral.mtu() << std::endl;
}

std::optional<ESP32_AD5933> esp32_ad5933 = std::nullopt;

namespace BLE_Payloads {
    //std::atomic<std::shared_ptr<std::string>> rx_payload;
}
