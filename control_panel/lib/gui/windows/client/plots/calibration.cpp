#include <algorithm>

#include <imgui_internal.h>
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
                    if(ImGui::BeginTabItem("CALIBRATION_DATA")) {
                        draw_calibration_data();
                        ImGui::EndTabItem(); 
                    }
                    if(ImGui::BeginTabItem("CALCULATED_DATA")) {
                        draw_calculated_data();
                        ImGui::EndTabItem(); 
                    }
                    if(ImGui::BeginTabItem("RAW_DATA")) {
                        draw_raw_data();
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
                if(ImPlot::BeginPlot("Calibration Raw Real Data", ImVec2(-1, 0), ImPlotFlags_NoLegend)) {
                    if(firsts.raw) {
                        ImPlot::SetupAxes("f [Hz]", "Real Data [1/Ohm]", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "Real Data [1/Ohm]");
                    }
                    ImPlot::PlotLine("Real Data", vectors.freq.data(), vectors.raw_real_data.data(), std::min(vectors.freq.size(), vectors.raw_imag_data.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Calibration Raw Imag Data", ImVec2(-1, 0), ImPlotFlags_NoLegend)) {
                    if(firsts.raw) {
                        ImPlot::SetupAxes("f [Hz]", "Imag Data [1/Ohm]", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.raw = false;
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "Imag Data [1/Ohm]");
                    }
                    ImPlot::PlotLine("Real Data", vectors.freq.data(), vectors.raw_imag_data.data(), std::min(vectors.freq.size(), vectors.raw_imag_data.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
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
                if(ImPlot::BeginPlot("Calibration Calculated Magnitude", ImVec2(-1, 0), ImPlotFlags_NoLegend)) {
                    if(firsts.calculated) {
                        ImPlot::SetupAxes("f [Hz]", "Raw Magnitude [1/Ohm]", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "Raw Magnitude [1/Ohm]");
                    }
                    ImPlot::PlotLine("Raw Magnitude", vectors.freq.data(), vectors.raw_magnitude.data(), std::min(vectors.freq.size(), vectors.raw_magnitude.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Calibration Calculated Phase", ImVec2(-1, 0), ImPlotFlags_NoLegend)) {
                    if(firsts.calculated) {
                        ImPlot::SetupAxes("f [Hz]", "Raw Phase [rad]", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "Raw Phase [rad]");
                        firsts.calculated = false;
                    }
                    ImPlot::PlotLine("Raw Phase", vectors.freq.data(), vectors.raw_phase.data(), std::min(vectors.freq.size(), vectors.raw_phase.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
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
                if(ImPlot::BeginPlot("Calibration Gain Factor", ImVec2(-1, 0), ImPlotFlags_NoLegend)) {
                    if(firsts.calibration) {
                        ImPlot::SetupAxes("f [Hz]", "Gain Factor", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "Gain Factor");
                    }
                    ImPlot::PlotLine("Gain Factor", vectors.freq.data(), vectors.gain_factor.data(), std::min(vectors.freq.size(), vectors.gain_factor.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Calibration System Phase", ImVec2(-1, 0), ImPlotFlags_NoLegend)) {
                    if(firsts.calibration) {
                        ImPlot::SetupAxes("f [Hz]", "System Phase [rad]", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.calibration = false;
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "System Phase [rad]");
                    }
                    ImPlot::PlotLine("System Phase", vectors.freq.data(), vectors.system_phase.data(), std::min(vectors.freq.size(), vectors.system_phase.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
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