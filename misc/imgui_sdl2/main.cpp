#include <thread>
#include <chrono>
#include <iostream>

#include "SDL2/SDL_main.h"
#include "simpleble/SimpleBLE.h"

#include "ble.hpp"
#include "imgui_sdl.hpp"

int main(int argc, char* argv[]) {
    std::optional<SimpleBLE::Peripheral> esp32_ret = std::nullopt;
    std::thread imgui_sdl_thread(ImGuiSDL::loop);
    if((esp32_ret = find_esp32_ad5933()) != std::nullopt) {
        esp32_ad5933 = ESP32_AD5933 { esp32_ret.value() };
        if(esp32_ad5933.value().connect() == false) {
            return -1;
        }
    }
    imgui_sdl_thread.join();
    return 0;
}