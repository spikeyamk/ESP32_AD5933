#include "default.hpp"

namespace Default {
    const size_t max_file_size { 1024 * 1024 };
    const AD5933::Config config {};
    const std::array<AD5933::Calibration<float>, 3> calibration {
        AD5933::Calibration<float> { 19764204.0, -1.2279561758041382 },
        AD5933::Calibration<float> { 19768642.0, -1.227928876876831  },
        AD5933::Calibration<float> { 19763664.0, -1.2277315855026245 }
    };
}