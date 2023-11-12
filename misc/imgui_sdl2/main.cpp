#include <thread>
#include <chrono>
#include <iostream>

#include "SDL2/SDL_main.h"
#include "simpleble/SimpleBLE.h"

#include "ble.hpp"
#include "imgui_sdl.hpp"

int main(int argc, char* argv[]) {
    std::thread imgui_sdl_thread(ImGuiSDL::loop);
    std::optional<SimpleBLE::Peripheral> esp32_ret = std::nullopt;
    if((esp32_ret = find_esp32_ad5933()) != std::nullopt) {
        esp32_ad5933 = ESP32_AD5933 { esp32_ret.value() };
        if(esp32_ad5933.value().connect() == false) {
            std::cout << "BLE: could not connect to ESP32_AD5933\n";
        }
        esp32_ad5933.value().subscribe_to_body_composition_measurement_notify();
    }
    imgui_sdl_thread.join();
    return 0;
}