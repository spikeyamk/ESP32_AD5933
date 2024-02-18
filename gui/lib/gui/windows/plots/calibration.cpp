#include <algorithm>

#include "imgui_internal.h"
#include "implot.h"

#include "gui/boilerplate.hpp"

#include "gui/windows/plots/calibration.hpp"

namespace GUI {
    namespace Windows {
        namespace Plots {
            Calibration::Calibration(size_t index) :
                index { index }
            {
                name.append(std::to_string(index));
            }

            void Calibration::draw(bool& enable, const ImGuiID side_id) {
                if(first) {
                    ImGui::DockBuilderDockWindow(name.c_str(), side_id);
                }

                if(ImGui::Begin(name.c_str(), &enable) == false) {
                    ImGui::End();
                    return;
                }

                if(first) {
                    ImGui::End();
                    first = false;
                    return;
                }

                if(ImGui::BeginTabBar("Calibration_PlotsBar")) {
                    if(ImGui::BeginTabItem("RAW_DATA")) {
                        draw_raw_data();
                        ImGui::EndTabItem(); 
                    }
                    if(ImGui::BeginTabItem("CALCULATED_DATA")) {
                        draw_calculated_data();
                        ImGui::EndTabItem(); 
                    }
                    if(ImGui::BeginTabItem("CALIBRATION_DATA")) {
                        draw_calibration_data();
                        ImGui::EndTabItem(); 
                    }
                    ImGui::EndTabBar();
                }
                ImGui::End();
            }

            void Calibration::update_vectors(
                const AD5933::Config& config,
                const std::vector<AD5933::Data>& raw_calibration,
                const std::vector<AD5933::Calibration<float>>& calibration
            ) {
                vectors.freq = config.get_freq_vector<float>();

                vectors.raw_real_data.resize(raw_calibration.size());
                std::transform(raw_calibration.begin(), raw_calibration.end(), vectors.raw_real_data.begin(), [](const AD5933::Data &e) { return static_cast<float>(e.get_real_data()); });

                vectors.raw_imag_data.resize(raw_calibration.size());
                std::transform(raw_calibration.begin(), raw_calibration.end(), vectors.raw_imag_data.begin(), [](const AD5933::Data &e) { return static_cast<float>(e.get_imag_data()); });

                vectors.raw_magnitude.resize(raw_calibration.size());
                std::transform(raw_calibration.begin(), raw_calibration.end(), vectors.raw_magnitude.begin(), [](const AD5933::Data &e) { return e.get_raw_magnitude<float>(); });

                vectors.raw_phase.resize(raw_calibration.size());
                std::transform(raw_calibration.begin(), raw_calibration.end(), vectors.raw_phase.begin(), [](const AD5933::Data &e) { return e.get_raw_phase<float>(); });
                
                vectors.gain_factor.resize(calibration.size());
                std::transform(calibration.begin(), calibration.end(), vectors.gain_factor.begin(), [](const AD5933::Calibration<float> &e) { return static_cast<float>(e.get_gain_factor()); });

                vectors.system_phase.resize(calibration.size());
                std::transform(calibration.begin(), calibration.end(), vectors.system_phase.begin(), [](const AD5933::Calibration<float> &e) { return static_cast<float>(e.get_system_phase()); });
            }

            void Calibration::draw_raw_data() {
                if(ImPlot::BeginPlot("Calibration Raw Real Data")) {
                    ImPlot::SetupAxes("f [Hz]", "RAW_REAL_DATA");
                    ImPlot::PlotLine("REAL_DATA [1/Ohm]", vectors.freq.data(), vectors.raw_real_data.data(), std::min(vectors.freq.size(), vectors.raw_imag_data.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Calibration Raw Imag Data")) {
                    ImPlot::SetupAxes("f [Hz]", "RAW_IMAG_DATA");
                    ImPlot::PlotLine("IMAG_DATA [1/Ohm]", vectors.freq.data(), vectors.raw_imag_data.data(), std::min(vectors.freq.size(), vectors.raw_imag_data.size()));
                    ImPlot::EndPlot();
                }
            }

            void Calibration::draw_calculated_data() {
                if(ImPlot::BeginPlot("Calibration Calculated Magnitude")) {
                    ImPlot::SetupAxes("f [Hz]", "RAW_MAGNITUDE");
                    ImPlot::PlotLine("RAW_MAGNITUDE [1/Ohm]", vectors.freq.data(), vectors.raw_magnitude.data(), std::min(vectors.freq.size(), vectors.raw_magnitude.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Calibration Calculated Phase")) {
                    ImPlot::SetupAxes("f [Hz]", "RAW_PHASE");
                    ImPlot::PlotLine("RAW_PHASE [rad]", vectors.freq.data(), vectors.raw_phase.data(), std::min(vectors.freq.size(), vectors.raw_phase.size()));
                    ImPlot::EndPlot();
                }
            }

            void Calibration::draw_calibration_data() {
                if(ImPlot::BeginPlot("Calibration Gain Factor")) {
                    ImPlot::SetupAxes("f [Hz]", "GAIN_FACTOR");
                    ImPlot::PlotLine("GAIN_FACTOR", vectors.freq.data(), vectors.gain_factor.data(), std::min(vectors.freq.size(), vectors.gain_factor.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Calibration System Phase")) {
                    ImPlot::SetupAxes("f [Hz]", "SYSTEM_PHASE");
                    ImPlot::PlotLine("SYSTEM_PHASE [rad]", vectors.freq.data(), vectors.system_phase.data(), std::min(vectors.freq.size(), vectors.system_phase.size()));
                    ImPlot::EndPlot();
                }
            }
        }
    }
}