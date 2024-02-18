#pragma once

#include <string>
#include <cstddef>
#include <vector>

#include "imgui.h"
#include <SDL3/SDL.h>

#include "ad5933/data/data.hpp"
#include "ad5933/measurement/measurement.hpp"

namespace GUI {
    namespace Windows {
        namespace Plots {
            class Measurement {
                size_t index;
                std::string name { "Measurement Plots##" };
                bool first { true };
                struct Raw {
                    std::vector<float> real_data;
                    std::vector<float> imag_data;
                    std::vector<float> magnitude;
                    std::vector<float> phase;
                };
                struct Corrected {
                    std::vector<float> impedance;
                    std::vector<float> phase;
                    std::vector<float> resistance;
                    std::vector<float> reactance;
                };
                struct Vectors {
                    std::vector<float> freq;
                    Raw raw;
                    Corrected corrected;
                };
                Vectors vectors;
            public:
                Measurement() = default;
                Measurement(size_t index);
                void draw(bool& enable, const ImGuiID side_id);
                void update_vectors(
                    const std::vector<float>& freq,
                    const std::vector<AD5933::Data>& raw_measurement,
                    const std::vector<AD5933::Measurement<float>>& measurement
                );
            private:
                void draw_raw_data();
                void draw_calculated_data();
                void draw_corrected_gon_data();
                void draw_corrected_alg_data();
            };
        }
    }
}