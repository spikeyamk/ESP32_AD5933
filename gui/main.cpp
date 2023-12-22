#include <thread>
#include <chrono>
#include <iostream>

#include "ble_client/ble_client.hpp"
#include "gui/imgui_sdl.hpp"

void gui_main() {
    std::optional<ESP32_AD5933> esp32_ad5933 { std::nullopt };
    bool done = false;
    std::thread imgui_sdl_thread(GUI::run, std::ref(esp32_ad5933), std::ref(done));
    std::optional<SimpleBLE::Peripheral> esp32_ret { std::nullopt };
    if((esp32_ret = find_esp32_ad5933(done)).has_value()) {
        esp32_ad5933 = ESP32_AD5933 { esp32_ret.value() };
        if(esp32_ad5933.value().connect() == false) {
            std::cout << "BLE: could not connect to ESP32_AD5933\n";
        } else {
            esp32_ad5933.value().subscribe_to_body_composition_measurement_notify();
        }
    }
    imgui_sdl_thread.join();
}

int main(int argc, char* argv[]) {
    gui_main();
    return 0;
}
