#pragma once

#include <variant>
#include <array>
#include <string>
#include <string_view>

#include <algorithm>
#include <cstring>

namespace BLE_Client {
    namespace StateMachines {
        namespace Adapter {
            namespace Events {
                struct turn_on{};
                struct start_discovery{};
                struct stop_discovery{};

                struct connect {
                private:
                    std::array<char, 18> p_address { 0 };
                public:
                    connect(const std::string_view& in_address);
                    const std::string_view get_address() const;
                private:
                    const std::string get_address_dots_instead_of_colons() const;
                public:
                    const std::string get_measurement_name() const;
                    const std::string get_information_name() const;
                };

                using T_Variant = std::variant<
                    turn_on,
                    start_discovery,
                    stop_discovery,
                    connect
                >;
            }
        }
    }
}