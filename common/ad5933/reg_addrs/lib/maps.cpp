#include "ad5933/reg_addrs/reg_addrs.hpp"

#include "ad5933/reg_addrs/maps.hpp"

namespace AD5933 {
    namespace RegAddrs {
        std::ostream& operator<<(std::ostream &os, const RW e) {
            os << get_map_str(e);
            return os;
        }

        std::ostream& operator<<(std::ostream &os, const RW_RO e) {
            os << get_map_str(e);
            return os;
        }
    }
}