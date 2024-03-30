#include <cmath>

#include "imgui_internal.h"
#include <utf/utf.hpp>

#include "gui/boilerplate.hpp"
#include "json/graph.hpp"
#include "implot_custom/setup_axis_label.hpp"

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
                                draw_save_corrected_gon_data();
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CORRECTED_ALG_DATA")) {
                                draw_save_corrected_alg_data();
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

            void Auto::update_send_vectors(const Windows::Auto::Point& send_point) {
                update_vectors(send_point, send_vectors);
            }

            void Auto::update_vectors(const Windows::Auto::Point& point, Vectors& vectors) {
                vectors.time.push_back(point.time);
                vectors.impedance.push_back(point.auto_meas.impedance);
                vectors.phase.push_back(point.auto_meas.phase);
                vectors.resistance.push_back(point.auto_meas.impedance * std::cos(point.auto_meas.phase));
                vectors.reactance.push_back(point.auto_meas.impedance * std::sin(point.auto_meas.phase));
            }

            void Auto::draw_send_corrected_gon_data() {
                if(ImPlot::BeginPlot("Auto Send Measurement Corrected Data")) {
                    if(firsts.send.corrected_gon) {
                        ImPlot::SetupAxes("Time", "IMPEDANCE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("Time", "IMPEDANCE");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    ImPlot::PlotLine("IMPEDANCE [Ohm]", send_vectors.time.data(), send_vectors.impedance.data(), std::min(send_vectors.time.size(), send_vectors.impedance.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Auto Send Measurement Calculated Phase")) {
                    if(firsts.send.corrected_gon) {
                        ImPlot::SetupAxes("Time", "PHASE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.send.corrected_gon = false;
                    } else {
                        ImPlot::SetupAxesLabels("Time", "PHASE");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    ImPlot::PlotLine("PHASE [rad]", send_vectors.time.data(), send_vectors.phase.data(), std::min(send_vectors.time.size(), send_vectors.phase.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    static constexpr char graph_name[] { "auto_send_corrected_gon" };
                    ns::CorrectedGonGraph2D_File<graph_name, double, ns::ValueNames::unix_timestamp> graph_file {};
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
                if(ImPlot::BeginPlot("Auto Send Measurement Resistance Data")) {
                    if(firsts.send.corrected_alg) {
                        ImPlot::SetupAxes("Time", "RESISTANCE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("Time", "RESISTANCE");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    ImPlot::PlotLine("RESISTANCE [Ohm]", send_vectors.time.data(), send_vectors.resistance.data(), std::min(send_vectors.time.size(), send_vectors.resistance.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Auto Send Measurement Reactance Data")) {
                    if(firsts.send.corrected_alg) {
                        ImPlot::SetupAxes("Time", "REACTANCE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.send.corrected_alg = false;
                    } else {
                        ImPlot::SetupAxesLabels("Time", "REACTANCE");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    ImPlot::PlotLine("REACTANCE [Ohm]", send_vectors.time.data(), send_vectors.reactance.data(), std::min(send_vectors.time.size(), send_vectors.reactance.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
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
        
            void Auto::update_save_vectors(const Windows::Auto::Point& save_point) {
                save_vectors = Vectors {};
                update_vectors(save_point, save_vectors);
            }

            void Auto::draw_save_corrected_gon_data() {
                if(ImPlot::BeginPlot("Auto Save Measurement Corrected Data")) {
                    if(firsts.save.corrected_gon) {
                        ImPlot::SetupAxes("Time", "IMPEDANCE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("Time", "IMPEDANCE");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    ImPlot::PlotLine("IMPEDANCE [Ohm]", save_vectors.time.data(), save_vectors.impedance.data(), std::min(save_vectors.time.size(), save_vectors.impedance.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Auto Save Measurement Calculated Phase")) {
                    if(firsts.save.corrected_gon) {
                        ImPlot::SetupAxes("Time", "PHASE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.save.corrected_gon = false;
                    } else {
                        ImPlot::SetupAxesLabels("Time", "PHASE");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    ImPlot::PlotLine("PHASE [rad]", save_vectors.time.data(), save_vectors.phase.data(), std::min(save_vectors.time.size(), save_vectors.phase.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    static constexpr char graph_name[] { "auto_save_corrected_gon" };
                    ns::CorrectedGonGraph2D_File<graph_name, double, ns::ValueNames::unix_timestamp> graph_file {};
                    ns::load_graph2D_file(
                        save_vectors.time,
                        save_vectors.impedance,
                        save_vectors.phase,
                        graph_file
                    );
                    ns::save_to_fs(graph_file);
                }
            }

            void Auto::draw_save_corrected_alg_data() {
                if(ImPlot::BeginPlot("Auto Save Measurement Resistance Data")) {
                    if(firsts.save.corrected_alg) {
                        ImPlot::SetupAxes("Time", "RESISTANCE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("Time", "RESISTANCE");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    ImPlot::PlotLine("RESISTANCE [Ohm]", save_vectors.time.data(), save_vectors.resistance.data(), std::min(save_vectors.time.size(), save_vectors.resistance.size()));
                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot("Auto Save Measurement Reactance Data")) {
                    if(firsts.save.corrected_alg) {
                        ImPlot::SetupAxes("Time", "REACTANCE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.save.corrected_alg = false;
                    } else {
                        ImPlot::SetupAxesLabels("Time", "REACTANCE");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    ImPlot::PlotLine("REACTANCE [Ohm]", save_vectors.time.data(), save_vectors.reactance.data(), std::min(save_vectors.time.size(), save_vectors.reactance.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    static constexpr char graph_name[] { "auto_save_corrected_alg" };
                    ns::CorrectedAlgGraph2D_File<graph_name, double, ns::ValueNames::unix_timestamp> graph_file {};
                    ns::load_graph2D_file(
                        save_vectors.time,
                        save_vectors.resistance,
                        save_vectors.reactance,
                        graph_file
                    );
                    ns::save_to_fs(graph_file);
                }
            }
        }
    }
}
