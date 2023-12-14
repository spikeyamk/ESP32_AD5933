#pragma once

#include <optional>

#include "ble.hpp"

namespace ImGuiSDL {
    void loop(std::optional<ESP32_AD5933> &esp32_ad5933, bool &done);
}