#include <thread>
#include <chrono>
#include <iostream>

#include "simpleble/SimpleBLE.h"

#include "ble.hpp"
#include "imgui_sdl.hpp"
#include "magic_packets.hpp"

void calibrate_cb(std::optional<ESP32_AD5933> &esp32_ad5933) {
    esp32_ad5933.value().send(std::string(
        MagicPackets::FrequencySweep::Command::start.begin(),
        MagicPackets::FrequencySweep::Command::start.end()
    ));
};

void cli_only_main() {
    std::optional<ESP32_AD5933> esp32_ad5933 { std::nullopt };

    std::optional<SimpleBLE::Peripheral> esp32_ret { std::nullopt };
    if((esp32_ret = find_esp32_ad5933()).has_value()) {
        esp32_ad5933 = ESP32_AD5933 { esp32_ret.value() };
        if(esp32_ad5933.value().connect() == false) {
            std::cout << "BLE: could not connect to ESP32_AD5933\n";
        } else {
            esp32_ad5933.value().subscribe_to_body_composition_measurement_notify();
            calibrate_cb(esp32_ad5933);
            while(1) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
        }
    }
}

void gui_main() {
    std::optional<ESP32_AD5933> esp32_ad5933 { std::nullopt };
    std::thread imgui_sdl_thread(ImGuiSDL::loop, std::ref(esp32_ad5933));

    std::optional<SimpleBLE::Peripheral> esp32_ret { std::nullopt };
    if((esp32_ret = find_esp32_ad5933()).has_value()) {
        esp32_ad5933 = ESP32_AD5933 { esp32_ret.value() };
        if(esp32_ad5933.value().connect() == false) {
            std::cout << "BLE: could not connect to ESP32_AD5933\n";
        } else {
            esp32_ad5933.value().subscribe_to_body_composition_measurement_notify();
            calibrate_cb(esp32_ad5933);
            while(1) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
        }
    }
    imgui_sdl_thread.join();
}

int main() {
    cli_only_main();
    return 0;
}

/* Commented out SDL_main()
int main(int argc, char* argv[]) {
    return 0;
}
*/