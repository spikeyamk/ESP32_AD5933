#pragma once

#include "ad5933/config/config.hpp"
#include "ad5933/config/masks.hpp"

namespace AD5933 {
    namespace Config_Tests {
        int test_getters();
        int test_setters(const AD5933::Config &in_config);
        int test_same_setters();
        int test_different_setters();
        int test_to_array_to_config();
        int test_array();
    }
}