#pragma once

#include <string>
#include <algorithm>
#include <array>
#include <string_view>

namespace BLE_Client {
    namespace Discovery {
        class Device {
        private:
            std::array<char, 50> p_identifier { 0 };
            std::array<char, 18> p_address { 0 };
        public:
            std::string_view identifier { p_identifier.begin(), p_identifier.end() - 1 };
            std::string_view address { p_address.begin(), p_address.end() - 1 };
            bool connected = false;

            Device() = default;

            inline Device(std::string in_identifier, std::string in_address, bool connected) :
                connected{ connected }
            {
                std::copy(in_identifier.begin(), in_identifier.end(), p_identifier.begin());
                std::copy(in_address.begin(), in_address.end(), p_address.begin());
            }
        };
    }
}
