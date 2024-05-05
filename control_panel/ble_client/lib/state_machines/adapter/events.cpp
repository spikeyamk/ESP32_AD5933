#include <algorithm>
#include <cstring>

#include "ble_client/state_machines/adapter/events.hpp"

namespace BLE_Client {
    namespace StateMachines {
        namespace Adapter {
            namespace Events {
                connect::connect(const std::string_view& in_address) {
                    std::copy(in_address.begin(), in_address.end(), p_address.begin());
                }

                const std::string_view connect::get_address() const {
                    const size_t len { std::strlen(p_address.data()) };
                    const std::string_view ret { p_address.begin(), (len > p_address.size()) ? (p_address.end()) : (p_address.begin() + len) };
                    return ret;
                }

                const std::string connect::get_address_dots_instead_of_colons() const {
                    std::string ret { get_address() };
                    for(auto& e: ret) {
                        if(e == ':') {
                            e = '.';
                        }
                    }
                    return ret;
                }
                const std::string connect::get_measurement_name() const {
                    return get_address_dots_instead_of_colons() + ".measurement";
                }

                const std::string connect::get_information_name() const {
                    return get_address_dots_instead_of_colons() + ".information";
                }
            }
        }
    }
}