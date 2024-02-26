#include <cmath>

#include "imgui_internal.h"
#include "implot.h"
#include <utf/utf.hpp>

#include "gui/boilerplate.hpp"
#include "json/graph.hpp"

#include "gui/windows/client/plots/auto.hpp"

namespace GUI {
    namespace Windows {
        namespace Plots {
            Auto::Auto(const size_t index) :
                index { index }
            {
                name.append(utf::as_u8(std::to_string(index)));
            }

            void Auto::draw(bool& enable, const ImGuiID side_id) {
                if(first) {
                    ImGui::DockBuilderDockWindow((const char*) name.c_str(), side_id);
                    ImPlot::CreateContext();
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

                if(ImGui::BeginTabBar("AUTO_PLOTS")) {
                    if(ImGui::BeginTabItem("SEND")) {
                        if(ImGui::BeginTabBar("SEND_TAB_BAR")) {
                            if(ImGui::BeginTabItem("CORRECTED_GON_DATA")) {
                                draw_send_corrected_gon_data();
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CORRECTED_ALG_DATA")) {
                                draw_send_corrected_alg_data();
                                ImGui::EndTabItem();
                            }
                            ImGui::EndTabBar();
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("SAVE")) {
                        if(ImGui::BeginTabBar("SAVE_TAB_BAR")) {
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

            void Auto::update_send_vectors(std::queue<Windows::Auto::Point>& send_points) {
                const auto front_point { send_points.front() };
                send_vectors.time.push_back(front_point.time);
                send_vectors.impedance.push_back(front_point.auto_meas.impedance);
                send_vectors.phase.push_back(front_point.auto_meas.phase);
                send_vectors.resistance.push_back(front_point.auto_meas.impedance * std::cos(front_point.auto_meas.phase));
                send_vectors.reactance.push_back(front_point.auto_meas.impedance * std::sin(front_point.auto_meas.phase));
                send_points.pop();
            }

            void Auto::draw_send_corrected_gon_data() {
                if(ImPlot::BeginPlot("Auto Measurement Corrected Data")) {
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    ImPlot::SetupAxes("Time", "CORRECTED_IMPEDANCE");
                    ImPlot::PlotLine("IMPEDANCE [Ohm]", send_vectors.time.data(), send_vectors.impedance.data(), std::min(send_vectors.time.size(), send_vectors.impedance.size()));
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Auto Measurement Calculated Phase")) {
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    ImPlot::SetupAxes("Time", "CORRECTED_PHASE");
                    ImPlot::PlotLine("PHASE [rad]", send_vectors.time.data(), send_vectors.phase.data(), std::min(send_vectors.time.size(), send_vectors.phase.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_name[] { "auto_send_corrected_gon" };
                    ns::CorrectedAlgGraph2D_File<graph_name, double, ns::ValueNames::unix_timestamp> graph_file {};
                    ns::load_graph2D_file(
                        send_vectors.time,
                        send_vectors.impedance,
                        send_vectors.phase,
                        graph_file
                    );
                    ns::save_to_fs(graph_file);
                }
            }

            void Auto::draw_send_corrected_alg_data() {
                if(ImPlot::BeginPlot("Auto Measurement Resistance Data")) {
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    ImPlot::SetupAxes("Time", "RESISTANCE");
                    ImPlot::PlotLine("RESISTANCE [Ohm]", send_vectors.time.data(), send_vectors.resistance.data(), std::min(send_vectors.time.size(), send_vectors.resistance.size()));
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Auto Measurement Reactance Data")) {
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    ImPlot::SetupAxes("Time", "REACTANCE");
                    ImPlot::PlotLine("REACTANCE [Ohm]", send_vectors.time.data(), send_vectors.reactance.data(), std::min(send_vectors.time.size(), send_vectors.reactance.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_name[] { "auto_send_corrected_alg" };
                    ns::CorrectedAlgGraph2D_File<graph_name, double, ns::ValueNames::unix_timestamp> graph_file {};
                    ns::load_graph2D_file(
                        send_vectors.time,
                        send_vectors.resistance,
                        send_vectors.reactance,
                        graph_file
                    );
                    ns::save_to_fs(graph_file);
                }
            }
        }
    }
}
