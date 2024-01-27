#pragma once

#include <array>
#include <string_view>

namespace BLE_Client {
    namespace StateMachines {
        namespace Connector {
            namespace Events {
                struct connect {
                private:
                    std::array<char, 17> p_address;
                public:
                    std::string_view address { p_address.begin(), p_address.end() };
                    /*
                    inline connect(std::array<char, 17> address) :
                        p_address{ address }
                    {}
                    */
                };
            }
        }
    }
}