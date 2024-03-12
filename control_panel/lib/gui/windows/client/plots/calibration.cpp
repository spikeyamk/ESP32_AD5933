#include <algorithm>
#include <string_view>

#include "imgui_internal.h"
#include <nfd.hpp>
#include <utf/utf.hpp>

#include "gui/boilerplate.hpp"
#include "json/graph.hpp"
#include "implot_custom/setup_axis_label.hpp"

#include "gui/windows/client/plots/calibration.hpp"

namespace GUI {
    namespace Windows {
        namespace Plots {
            Calibration::Calibration(size_t index) :
                index { index }
            {
                name.append(utf::as_u8(std::to_string(index)));
            }

            void Calibration::draw(bool& enable, const ImGuiID side_id) {
                if(first) {
                    ImGui::DockBuilderDockWindow((const char*) name.c_str(), side_id);
                }

                if(ImGui::Begin((const char*) name.c_str(), &enable, ImGuiWindowFlags_NoMove) == false) {
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
                const AD5933::Config config,
                const std::vector<AD5933::Data> raw_calibration,
                const std::vector<AD5933::Calibration<float>> calibration
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
                    if(firsts.raw) {
                        ImPlot::SetupAxes("f [Hz]", "REAL_DATA", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "REAL_DATA");
                    }
                    ImPlot::PlotLine("REAL_DATA [1/Ohm]", vectors.freq.data(), vectors.raw_real_data.data(), std::min(vectors.freq.size(), vectors.raw_imag_data.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Calibration Raw Imag Data")) {
                    if(firsts.raw) {
                        ImPlot::SetupAxes("f [Hz]", "IMAG_DATA", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.raw = false;
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "IMAG_DATA");
                    }
                    ImPlot::PlotLine("IMAG_DATA [1/Ohm]", vectors.freq.data(), vectors.raw_imag_data.data(), std::min(vectors.freq.size(), vectors.raw_imag_data.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_name[] { "calibration_raw" };
                    ns::RawDataGraph2D_File<graph_name, float, ns::ValueNames::freq> graph_file {};
                    ns::load_graph2D_file(
                        vectors.freq,
                        vectors.raw_real_data,
                        vectors.raw_imag_data,
                        graph_file
                    );
                    ns::save_to_fs(graph_file);
                }
            }

            void Calibration::draw_calculated_data() {
                if(ImPlot::BeginPlot("Calibration Calculated Magnitude")) {
                    if(firsts.calculated) {
                        ImPlot::SetupAxes("f [Hz]", "RAW_MAGNITUDE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "RAW_MAGNITUDE");
                    }
                    ImPlot::PlotLine("RAW_MAGNITUDE [1/Ohm]", vectors.freq.data(), vectors.raw_magnitude.data(), std::min(vectors.freq.size(), vectors.raw_magnitude.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Calibration Calculated Phase")) {
                    if(firsts.calculated) {
                        ImPlot::SetupAxes("f [Hz]", "RAW_PHASE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "RAW_PHASE");
                        firsts.calculated = false;
                    }
                    ImPlot::PlotLine("RAW_PHASE [rad]", vectors.freq.data(), vectors.raw_phase.data(), std::min(vectors.freq.size(), vectors.raw_phase.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_name[] { "calibration_calculated" };
                    ns::CalculatedGraph2D_File<graph_name, float, ns::ValueNames::freq> graph_file {};
                    ns::load_graph2D_file(
                        vectors.freq,
                        vectors.raw_magnitude,
                        vectors.raw_phase,
                        graph_file
                    );
                    ns::save_to_fs(graph_file);
                }
            }

            void Calibration::draw_calibration_data() {
                if(ImPlot::BeginPlot("Calibration Gain Factor")) {
                    if(firsts.calibration) {
                        ImPlot::SetupAxes("f [Hz]", "GAIN_FACTOR", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "GAIN_FACTOR");
                    }
                    ImPlot::PlotLine("GAIN_FACTOR", vectors.freq.data(), vectors.gain_factor.data(), std::min(vectors.freq.size(), vectors.gain_factor.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Calibration System Phase")) {
                    if(firsts.calibration) {
                        ImPlot::SetupAxes("f [Hz]", "SYSTEM_PHASE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.calibration = false;
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "SYSTEM_PHASE");
                    }
                    ImPlot::PlotLine("SYSTEM_PHASE [rad]", vectors.freq.data(), vectors.system_phase.data(), std::min(vectors.freq.size(), vectors.system_phase.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_name[] { "calibration_calibration" };
                    ns::CalibrationGraph2D_File<graph_name, float, ns::ValueNames::freq> graph_file {};
                    ns::load_graph2D_file(
                        vectors.freq,
                        vectors.gain_factor,
                        vectors.system_phase,
                        graph_file
                    );
                    ns::save_to_fs(graph_file);
                }
            }
        }
    }
}