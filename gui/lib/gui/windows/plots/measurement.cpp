#include <algorithm>

#include "imgui_internal.h"
#include "implot.h"

#include "gui/boilerplate.hpp"

#include "gui/windows/plots/measurement.hpp"

namespace GUI {
    namespace Windows {
        namespace Plots {
            Measurement::Measurement(size_t index) :
                index { index }
            {
                name.append(std::to_string(index));
            }

            void Measurement::draw(bool& enable, const ImGuiID side_id) {
                if(first) {
                    ImGui::DockBuilderDockWindow(name.c_str(), side_id);
                    ImPlot::CreateContext();
                    /*
                    const float scale = Boilerplate::get_scale();
                    ImPlot::GetStyle().PlotDefaultSize.x *= scale;
                    ImPlot::GetStyle().PlotDefaultSize.y *= scale;
                    */
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

                if(ImGui::BeginTabBar("MEASUREMENT_PLOTS")) {
                    if(ImGui::BeginTabItem("SINGLE")) {
                        if(ImGui::BeginTabBar("SINGLE_TAB_BAR")) {
                            if(ImGui::BeginTabItem("RAW_DATA")) {
                                draw_raw_data();
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CALCULATED_DATA")) {
                                draw_calculated_data();
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CORRECTED_GON_DATA")) {
                                draw_corrected_gon_data();
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CORRECTED_ALG_DATA")) {
                                draw_corrected_alg_data();
                                ImGui::EndTabItem();
                            }
                            ImGui::EndTabBar();
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("PERIODIC")) {
                        if(ImGui::BeginTabBar("PERIODIC_TAB_BAR")) {
                            if(ImGui::BeginTabItem("RAW_DATA")) {
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CALCULATED_DATA")) {
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CORRECTED_GON_DATA")) {
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CORRECTED_ALG_DATA")) {
                                ImGui::EndTabItem();
                            }
                            ImGui::EndTabBar();
                        }
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::End();
            }

            void Measurement::update_vectors(
                const std::vector<float>& freq,
                const std::vector<AD5933::Data>& raw_measurement,
                const std::vector<AD5933::Measurement<float>>& measurement
            ) {
                vectors.freq = freq;

                vectors.raw.real_data.resize(raw_measurement.size());
                std::transform(raw_measurement.begin(), raw_measurement.end(), vectors.raw.real_data.begin(), [](const AD5933::Data &e) { return static_cast<float>(e.get_real_data()); });

                vectors.raw.imag_data.resize(raw_measurement.size());
                std::transform(raw_measurement.begin(), raw_measurement.end(), vectors.raw.imag_data.begin(), [](const AD5933::Data &e) { return e.get_raw_magnitude<float>(); });

                vectors.raw.magnitude.resize(raw_measurement.size());
                std::transform(raw_measurement.begin(), raw_measurement.end(), vectors.raw.magnitude.begin(), [](const AD5933::Data &e) { return e.get_raw_magnitude<float>(); });

                vectors.raw.phase.resize(raw_measurement.size());
                std::transform(raw_measurement.begin(), raw_measurement.end(), vectors.raw.phase.begin(), [](const AD5933::Data &e) { return e.get_raw_phase<float>(); });

                vectors.corrected.impedance.resize(measurement.size());
                std::transform(measurement.begin(), measurement.end(), vectors.corrected.impedance.begin(), [](const AD5933::Measurement<float> &e) { return e.get_magnitude(); });               

                vectors.corrected.phase.resize(measurement.size());
                std::transform(measurement.begin(), measurement.end(), vectors.corrected.phase.begin(), [](const AD5933::Measurement<float> &e) { return e.get_phase(); });

                vectors.corrected.resistance.resize(measurement.size());
                std::transform(measurement.begin(), measurement.end(), vectors.corrected.resistance.begin(), [](const AD5933::Measurement<float> &e) { return e.get_resistance(); });

                vectors.corrected.reactance.resize(measurement.size());
                std::transform(measurement.begin(), measurement.end(), vectors.corrected.reactance.begin(), [](const AD5933::Measurement<float> &e) { return e.get_reactance(); });
            }

            void Measurement::draw_raw_data() {
                if(ImPlot::BeginPlot("Measurement Raw Real Data")) {
                    ImPlot::SetupAxes("f [Hz]", "REAL_DATA");
                    ImPlot::PlotLine("REAL_DATA [1/Ohm]", vectors.freq.data(), vectors.raw.real_data.data(), std::min(vectors.freq.size(), vectors.raw.real_data.size()));
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Measurement Raw Imag Data")) {
                    ImPlot::SetupAxes("f [Hz]", "IMAG_DATA");
                    ImPlot::PlotLine("IMAG_DATA [1/Ohm]", vectors.freq.data(), vectors.raw.imag_data.data(), std::min(vectors.freq.size(), vectors.raw.imag_data.size()));
                    ImPlot::EndPlot();
                }
            }

            void Measurement::draw_calculated_data() {
                if(ImPlot::BeginPlot("Measurement Calculated Magnitude")) {
                    ImPlot::SetupAxes("f [Hz]", "RAW_MAGNITUDE");
                    ImPlot::PlotLine("RAW_MAGNITUDE [1/Ohm]", vectors.freq.data(), vectors.raw.magnitude.data(), std::min(vectors.freq.size(), vectors.raw.magnitude.size()));
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Measurement Calculated Phase")) {
                    ImPlot::SetupAxes("f [Hz]", "RAW_PHASE");
                    ImPlot::PlotLine("RAW_PHASE [rad]", vectors.freq.data(), vectors.raw.phase.data(), std::min(vectors.freq.size(), vectors.raw.phase.size()));
                    ImPlot::EndPlot();
                }
            }

            void Measurement::draw_corrected_gon_data() {
                if(ImPlot::BeginPlot("Measurement Corrected Data")) {
                    ImPlot::SetupAxes("f [Hz]", "CORRECTED_IMPEDANCE");
                    ImPlot::PlotLine("CORRECTED_IMPEDANCE [Ohm]", vectors.freq.data(), vectors.corrected.impedance.data(), std::min(vectors.freq.size(), vectors.corrected.impedance.size()));
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Measurement Calculated Phase")) {
                    ImPlot::SetupAxes("f [Hz]", "CORRECTED_PHASE");
                    ImPlot::PlotLine("CORRECTED_PHASE [rad]", vectors.freq.data(), vectors.corrected.phase.data(), std::min(vectors.freq.size(), vectors.corrected.phase.size()));
                    ImPlot::EndPlot();
                }
            }

            void Measurement::draw_corrected_alg_data() {
                if(ImPlot::BeginPlot("Measurement Resistance Data")) {
                    ImPlot::SetupAxes("f [Hz]", "RESISTANCE");
                    ImPlot::PlotLine("RESISTANCE [Ohm]", vectors.freq.data(), vectors.corrected.resistance.data(), std::min(vectors.freq.size(), vectors.corrected.resistance.size()));
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Measurement Reactance Data")) {
                    ImPlot::SetupAxes("f [Hz]", "REACTANCE");
                    ImPlot::PlotLine("REACTANCE [Ohm]", vectors.freq.data(), vectors.corrected.reactance.data(), std::min(vectors.freq.size(), vectors.corrected.reactance.size()));
                    ImPlot::EndPlot();
                }
            }
        }
    }
}