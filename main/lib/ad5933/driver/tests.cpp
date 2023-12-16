#include <array>
#include <cstdint>

#include "ad5933/config/config.hpp"
#include "ad5933/config/tests.hpp"

#include "ad5933/driver/tests.hpp"

namespace AD5933 {
    namespace Driver_Tests {
        int runtime_test_driver(const Driver &driver) {
            const auto lambda = [&driver](Config config) -> int {
                if(driver.program_all_registers(config.to_raw_array()) == false) {
                    return -1;
                }

                const auto ret { driver.block_read_register<12, RegAddrs::RW_RO::ControlHB>() };
                if(ret.has_value() == false) {
                    return -2;
                }

                if(ret.value() != config.to_raw_array()) {
                    return -3;
                }

                config.print();
                Config(ret.value()).print();
                driver.print_all_registers();
                return 0;
            };

            const auto ret1 = lambda(Config_Tests::test_config);
            if(ret1 != 0) {
                return ret1 - 10;
            }

            const auto ret2 = lambda(Config_Tests::another_test_config);
            if(ret2 != 0) {
                return ret2 - 20;
            }

            return 0;
        }
    }
}