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
            bool connected = false;

            Device() = default;

            inline Device(const std::string& in_identifier, const std::string& in_address, bool connected) :
                connected{ connected }
            {
                std::copy(in_identifier.begin(), in_identifier.end(), p_identifier.begin());
                std::copy(in_address.begin(), in_address.end(), p_address.begin());
            }

            std::string_view get_identifier() const {
                return get_string_view(p_identifier);
            }

            std::string_view get_address() const {
                return get_string_view(p_address);
            }

            std::string get_address_with_dots_instead_of_colons() const {
                std::string ret { get_address() };
                for(auto& e: ret) {
                    if(e == ':') {
                        e = '.';
                    }
                }
                return ret;
            }
        private:
            template<typename T>
            std::string_view get_string_view(const T& array) const {
                const size_t len = std::strlen(array.data());
                const std::string_view ret { array.begin(), (len > array.size()) ? (array.end()) : (array.begin() + len) };
                return ret;
            }
        };
    }
}
