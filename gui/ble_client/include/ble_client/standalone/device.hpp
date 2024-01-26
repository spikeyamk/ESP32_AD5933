#pragma once

#include <string>
#include <algorithm>
#include <array>

namespace BLE_Client {
    namespace Discovery {
        class Device {
        public:
            std::array<char, 31> identifier { 0 };
            std::array<char, 18> address { 0 };
            bool connected = false;

            Device() = default;

            inline Device(std::string in_identifier, std::string in_address, bool connected) :
                connected{ connected }
            {
                std::copy(in_identifier.begin(), in_identifier.end(), identifier.begin());
                std::copy(in_address.begin(), in_address.end(), address.begin());
            }
        };
    }
}
