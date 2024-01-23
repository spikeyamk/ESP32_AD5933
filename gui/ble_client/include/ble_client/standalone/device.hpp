#pragma once

#include <string>
#include <algorithm>

namespace BLE_Client {
    namespace Discovery {
        class Device {
        public:
            char identifier[31] { 0 };
            char address[18] { 0 };
            bool connected = false;

            Device() = default;

            inline Device(std::string in_identifier, std::string in_address, bool connected) :
                connected{ connected }
            {
                std::copy(in_identifier.begin(), in_identifier.end(), identifier);
                std::copy(in_address.begin(), in_address.end(), address);
            }
        };
    }
}
