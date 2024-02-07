#pragma once

#include <vector>
#include <string>

#include "imgui.h"

#include "ad5933/uint_types.hpp"
#include "ad5933/data/data.hpp"
#include "ad5933/calibration/calibration.hpp"
#include "ad5933/config/config.hpp"

namespace GUI {
    namespace Windows {
        namespace Plots {
            class Calibration {
                size_t index;
                std::string name { "Calibration Plots##" };
                struct Vectors {
                    std::vector<float> freq;
                    std::vector<float> raw_real_data;
                    std::vector<float> raw_imag_data;
                    std::vector<float> raw_magnitude;
                    std::vector<float> raw_phase;
                    std::vector<float> gain_factor;
                    std::vector<float> system_phase;
                };
                bool first { true };
            public:
                Vectors vectors;
                Calibration() = default;
                Calibration(size_t index);
                void draw(bool& enable, const ImGuiID side_id);
                void update_vectors(
                    const AD5933::Config& config,
                    const std::vector<AD5933::Data>& raw_calibration,
                    const std::vector<AD5933::Calibration<float>>& calibration
                );
            private:
                void draw_raw_data();
                void draw_calculated_data();
                void draw_calibration_data();
            };
        }
    }
}