#pragma once

#include <array>
#include <string_view>
#include <algorithm>
#include <cstring>

namespace BLE_Client {
    namespace StateMachines {
        namespace Connector {
            namespace Events {
                struct connect {
                private:
                    std::array<char, 18> p_address { 0 };
                public:
                    inline connect(const std::string_view& in_address) {
                        std::copy(in_address.begin(), in_address.end(), p_address.begin());
                    }

                    inline const std::string_view get_address() const {
                        const size_t len = std::strlen(p_address.data());
                        const std::string_view ret { p_address.begin(), (len > p_address.size()) ? (p_address.end()) : (p_address.begin() + len) };
                        return ret;
                    }

                    inline const std::string get_address_dots_instead_of_colons() const {
                        std::string ret { get_address() };
                        for(auto& e: ret) {
                            if(e == ':') {
                                e = '.';
                            }
                        }
                        return ret;
                    }
                };
            }
        }
    }
}