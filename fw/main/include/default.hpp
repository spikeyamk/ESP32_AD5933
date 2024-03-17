#include <array>

#include "ad5933/config/config.hpp"
#include "ad5933/calibration/calibration.hpp"

namespace Default {
    extern const size_t max_file_size;
    extern const AD5933::Config config;
    extern const std::array<AD5933::Calibration<float>, 3> calibration;
}