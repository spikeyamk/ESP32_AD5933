#pragma once

#include <optional>

#include "ble.hpp"

namespace ImGuiSDL {
    void loop(std::optional<ESP32_AD5933> &in_esp32_ad5933);
}