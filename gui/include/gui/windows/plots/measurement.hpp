#pragma once

#include <string>
#include <cstddef>
#include <vector>

#include "imgui.h"

#include "ad5933/data/data.hpp"
#include "ad5933/measurement/measurement.hpp"
#include "gui/windows/measure.hpp"

namespace GUI {
    namespace Windows {
        namespace Plots {
            class Measurement {
            private:
                size_t index;
                std::string name { "Measurement Plots##" };
                bool first { true };
            private:
                template<typename T>
                struct Raw {
                    std::vector<T> real_data;
                    std::vector<T> imag_data;
                    std::vector<T> magnitude;
                    std::vector<T> phase;
                };
                template<typename T>
                struct Corrected {
                    std::vector<T> impedance;
                    std::vector<T> phase;
                    std::vector<T> resistance;
                    std::vector<T> reactance;
                };
                struct SingleVectors {
                    std::vector<float> freq;
                    Raw<float> raw;
                    Corrected<float> corrected;
                };
                SingleVectors single_vectors {};
            private:
                struct TimedPoint {
                    Raw<double> raw;
                    Corrected<double> corrected;
                };
                struct PeriodicVectors {
                    std::vector<float> freq;
                    std::vector<double> time_points;
                    std::vector<TimedPoint> points;
                };
                PeriodicVectors periodic_vectors {};
            public:
                Measurement() = default;
                Measurement(size_t index);
                void draw(bool& enable, const ImGuiID side_id);
                void update_single_vectors(
                    const std::vector<float>& freq,
                    const std::vector<AD5933::Data>& raw_measurement,
                    const std::vector<AD5933::Measurement<float>>& measurement
                );
                void update_periodic_vectors(
                    const std::vector<float>& freq,
                    std::queue<Measure::PeriodicPoint>& periodic_points
                );
            private:
                void draw_single_raw_data();
                void draw_single_calculated_data();
                void draw_single_corrected_gon_data();
                void draw_single_corrected_alg_data();
            private:
                struct PeriodicIndexSelects {
                    uint32_t raw { 0 };
                    uint32_t calculated { 0 };
                    uint32_t corrected_gon { 0 };
                    uint32_t corrected_alg { 0 };
                };
                PeriodicIndexSelects periodic_index_selects {};
                struct PeriodicFreqSelects {
                    uint32_t raw { 0 };
                    uint32_t calculated { 0 };
                    uint32_t corrected_gon { 0 };
                    uint32_t corrected_alg { 0 };
                };
                PeriodicFreqSelects periodic_freq_selects {};
                void draw_freq_input(uint32_t& freq_select, uint32_t& index_select);
                void draw_periodic_raw_data();
                void draw_periodic_calculated_data();
                void draw_periodic_corrected_gon_data();
                void draw_periodic_corrected_alg_data();
            };
        }
    }
}