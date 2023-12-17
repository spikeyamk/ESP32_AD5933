#pragma once

#include <optional>

#include "ble.hpp"

namespace GUI {
    void run(std::optional<ESP32_AD5933> &esp32_ad5933, bool &done);
}