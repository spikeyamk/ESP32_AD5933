#include <algorithm>

#include "imgui_internal.h"
#include <utf/utf.hpp>

#include "imgui_custom/input_items.hpp"
#include "imgui_custom/char_filters.hpp"
#include "implot_custom/setup_axis_label.hpp"
#include "gui/boilerplate.hpp"
#include "json/graph.hpp"

#include "gui/windows/client/plots/measurement.hpp"

namespace GUI {
    namespace Windows {
        namespace Plots {
            Measurement::Measurement(size_t index) :
                index { index }
            {
                name.append(utf::as_u8(std::to_string(index)));
            }

            void Measurement::draw(bool& enable, const ImGuiID side_id) {
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

                if(ImGui::BeginTabBar("MEASUREMENT_PLOTS")) {
                    if(ImGui::BeginTabItem("SINGLE")) {
                        if(ImGui::BeginTabBar("SINGLE_TAB_BAR")) {
                            if(ImGui::BeginTabItem("RAW_DATA")) {
                                draw_single_raw_data();
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CALCULATED_DATA")) {
                                draw_single_calculated_data();
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CORRECTED_GON_DATA")) {
                                draw_single_corrected_gon_data();
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CORRECTED_ALG_DATA")) {
                                draw_single_corrected_alg_data();
                                ImGui::EndTabItem();
                            }
                            ImGui::EndTabBar();
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("PERIODIC")) {
                        if(ImGui::BeginTabBar("PERIODIC_TAB_BAR")) {
                            if(ImGui::BeginTabItem("RAW_DATA")) {
                                draw_periodic_raw_data();
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CALCULATED_DATA")) {
                                draw_periodic_calculated_data();
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CORRECTED_GON_DATA")) {
                                draw_periodic_corrected_gon_data();
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CORRECTED_ALG_DATA")) {
                                draw_periodic_corrected_alg_data();
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

            void Measurement::draw_freq_input(uint32_t& freq_select, uint32_t& index_select) {
                if(periodic_vectors.points.empty() == false) {
                    if(ImGui::Input_uint32_t_WithCallbackTextReadOnly(
                        "Frequency [Hz]",
                        &freq_select,
                        periodic_vectors.freq[1] - periodic_vectors.freq[0],
                        periodic_vectors.freq[1] - periodic_vectors.freq[0],
                        ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter,
                        ImGui::plus_minus_dot_char_filter_cb
                    )) {
                        if(freq_select < periodic_vectors.freq.front()) {
                            freq_select = periodic_vectors.freq.front();
                        } else if(freq_select > periodic_vectors.freq.back()) {
                            freq_select = periodic_vectors.freq.back();
                        } else {
                            for(size_t i = 0; i < periodic_vectors.freq.size(); i++) {
                                if(freq_select == periodic_vectors.freq[i]) {
                                    index_select = i;
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    ImGui::BeginDisabled();
                    ImGui::Input_uint32_t_WithCallbackTextReadOnly(
                        "Frequency [Hz]",
                        &freq_select,
                        0,
                        0,
                        ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter,
                        ImGui::plus_minus_dot_char_filter_cb
                    );
                    ImGui::EndDisabled();
                }
            }

            void Measurement::draw_periodic_raw_data() {
                draw_freq_input(periodic_freq_selects.raw, periodic_index_selects.raw);
                if(ImPlot::BeginPlot("Measurement Periodic Raw Real Data")) {
                    if(firsts.periodic.raw) {
                        ImPlot::SetupAxes("Time", "REAL_DATA", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("Time", "REAL_DATA");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    if(periodic_vectors.points.empty() == false) {
                        ImPlot::PlotLine("REAL_DATA [1/Ohm]", periodic_vectors.time_points.data(), periodic_vectors.points[periodic_index_selects.raw].raw.real_data.data(), std::min(periodic_vectors.time_points.size(), periodic_vectors.points[periodic_index_selects.raw].raw.real_data.size()));
                    }
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Measurement Perioidc Raw Imag Data")) {
                    if(firsts.periodic.raw) {
                        ImPlot::SetupAxes("Time", "IMAG_DATA", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.periodic.raw = false;
                    } else {
                        ImPlot::SetupAxesLabels("Time", "IMAG_DATA");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    if(periodic_vectors.points.empty() == false) {
                        ImPlot::PlotLine("IMAG_DATA [1/Ohm]", periodic_vectors.time_points.data(), periodic_vectors.points[periodic_index_selects.raw].raw.imag_data.data(), std::min(periodic_vectors.time_points.size(), periodic_vectors.points[periodic_index_selects.raw].raw.imag_data.size()));
                    }
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_file_name[] { "measurement_periodic_raw" };
                    ns::RawDataGraph3D_File<graph_file_name, ns::Graph3D_Names::zfreq_raw_data, ns::Freq, double, ns::ValueNames::unix_timestamp> graph3D_file;
                    const size_t wished_size { periodic_vectors.freq.size() };
                    graph3D_file.graph_3D.points.reserve(wished_size);
                    for(size_t i = 0; i < wished_size; i++) {
                        graph3D_file.graph_3D.points.push_back(
                            {
                                periodic_vectors.freq[i],
                            }
                        );
                        const size_t min { std::min(std::min(periodic_vectors.time_points.size(), periodic_vectors.points[i].raw.real_data.size()), periodic_vectors.points[i].raw.imag_data.size()) };
                        graph3D_file.graph_3D.points[i].graph_2D.points.reserve(min);
                        for(size_t j = 0; j < min; j++) {
                            graph3D_file.graph_3D.points[i].graph_2D.points.push_back(
                                {
                                    periodic_vectors.time_points[j],
                                    periodic_vectors.points[i].raw.real_data[j],
                                    periodic_vectors.points[i].raw.imag_data[j],
                                }
                            );
                        }
                    }

                    ns::save_to_fs(graph3D_file);
                }
            }

            void Measurement::draw_periodic_calculated_data() {
                draw_freq_input(periodic_freq_selects.calculated, periodic_index_selects.calculated);
                if(ImPlot::BeginPlot("Measurement Periodic Calculated Magnitude")) {
                    if(firsts.periodic.calculated) {
                        ImPlot::SetupAxes("Time", "RAW_MAGNITUDE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("Time", "RAW_MAGNITUDE");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    if(periodic_vectors.points.empty() == false) {
                        ImPlot::PlotLine("RAW_MAGNITUDE [1/Ohm]", periodic_vectors.time_points.data(), periodic_vectors.points[periodic_index_selects.calculated].raw.magnitude.data(), std::min(periodic_vectors.time_points.size(), periodic_vectors.points[periodic_index_selects.calculated].raw.magnitude.size()));
                    }
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Measurement Periodic Calculated Phase")) {
                    if(firsts.periodic.calculated) {
                        ImPlot::SetupAxes("Time", "RAW_PHASE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.periodic.calculated = false;
                    } else {
                        ImPlot::SetupAxesLabels("Time", "RAW_PHASE");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    if(periodic_vectors.points.empty() == false) {
                        ImPlot::PlotLine("RAW_PHASE [rad]", periodic_vectors.time_points.data(), periodic_vectors.points[periodic_index_selects.calculated].raw.phase.data(), std::min(periodic_vectors.time_points.size(), periodic_vectors.points[periodic_index_selects.calculated].raw.phase.size()));
                    }
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_file_name[] { "measurement_periodic_calculated" };
                    ns::CalculatedGraph3D_File<graph_file_name, ns::Graph3D_Names::zfreq_calculated, ns::Freq, double, ns::ValueNames::unix_timestamp> graph3D_file;
                    const size_t wished_size { periodic_vectors.freq.size() };
                    graph3D_file.graph_3D.points.reserve(wished_size);
                    for(size_t i = 0; i < wished_size; i++) {
                        graph3D_file.graph_3D.points.push_back(
                            {
                                periodic_vectors.freq[i],
                            }
                        );
                        const size_t min { std::min(std::min(periodic_vectors.time_points.size(), periodic_vectors.points[i].raw.magnitude.size()), periodic_vectors.points[i].raw.phase.size()) };
                        graph3D_file.graph_3D.points[i].graph_2D.points.reserve(min);
                        for(size_t j = 0; j < min; j++) {
                            graph3D_file.graph_3D.points[i].graph_2D.points.push_back(
                                {
                                    periodic_vectors.time_points[j],
                                    periodic_vectors.points[i].raw.magnitude[j],
                                    periodic_vectors.points[i].raw.phase[j],
                                }
                            );
                        }
                    }

                    ns::save_to_fs(graph3D_file);
                }
            }

            void Measurement::draw_periodic_corrected_gon_data() {
                draw_freq_input(periodic_freq_selects.corrected_gon, periodic_index_selects.corrected_gon);
                if(ImPlot::BeginPlot("Measurement Periodic Corrected Impedance")) {
                    if(firsts.periodic.corrected_gon) {
                        ImPlot::SetupAxes("Time", "IMPEDANCE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("Time", "IMPEDANCE");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    if(periodic_vectors.points.empty() == false) {
                        ImPlot::PlotLine("IMPEDANCE [Ohm]", periodic_vectors.time_points.data(), periodic_vectors.points[periodic_index_selects.corrected_gon].corrected.impedance.data(), std::min(periodic_vectors.time_points.size(), periodic_vectors.points[periodic_index_selects.corrected_gon].corrected.impedance.size()));
                    }
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Measurement Periodic Corrected Phase")) {
                    if(firsts.periodic.corrected_gon) {
                        ImPlot::SetupAxes("Time", "PHASE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.periodic.corrected_gon = false;
                    } else {
                        ImPlot::SetupAxesLabels("Time", "PHASE");
                    }

                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    if(periodic_vectors.points.empty() == false) {
                        ImPlot::PlotLine("PHASE [rad]", periodic_vectors.time_points.data(), periodic_vectors.points[periodic_index_selects.corrected_gon].corrected.phase.data(), std::min(periodic_vectors.time_points.size(), periodic_vectors.points[periodic_index_selects.corrected_gon].corrected.phase.size()));
                    }
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_file_name[] { "measurement_periodic_corrected_gon" };
                    ns::CorrectedGonGraph3D_File<graph_file_name, ns::Graph3D_Names::zfreq_corrected_gon, ns::Freq, double, ns::ValueNames::unix_timestamp> graph3D_file;
                    const size_t wished_size { periodic_vectors.freq.size() };
                    graph3D_file.graph_3D.points.reserve(wished_size);
                    for(size_t i = 0; i < wished_size; i++) {
                        graph3D_file.graph_3D.points.push_back(
                            {
                                periodic_vectors.freq[i],
                            }
                        );
                        const size_t min { std::min(std::min(periodic_vectors.time_points.size(), periodic_vectors.points[i].corrected.impedance.size()), periodic_vectors.points[i].corrected.phase.size()) };
                        graph3D_file.graph_3D.points[i].graph_2D.points.reserve(min);
                        for(size_t j = 0; j < min; j++) {
                            graph3D_file.graph_3D.points[i].graph_2D.points.push_back(
                                {
                                    periodic_vectors.time_points[j],
                                    periodic_vectors.points[i].corrected.impedance[j],
                                    periodic_vectors.points[i].corrected.phase[j],
                                }
                            );
                        }
                    }

                    ns::save_to_fs(graph3D_file);
                }
            }

            void Measurement::draw_periodic_corrected_alg_data() {
                draw_freq_input(periodic_freq_selects.corrected_alg, periodic_index_selects.corrected_alg);
                if(ImPlot::BeginPlot("Measurement Periodic Resistance Data")) {
                    if(firsts.periodic.corrected_alg) {
                        ImPlot::SetupAxes("Time", "RESISTANCE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("Time", "RESISTANCE");
                    }
                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    if(periodic_vectors.points.empty() == false) {
                        ImPlot::PlotLine("RESISTANCE [Ohm]", periodic_vectors.time_points.data(), periodic_vectors.points[periodic_index_selects.corrected_alg].corrected.resistance.data(), std::min(periodic_vectors.time_points.size(), periodic_vectors.points[periodic_index_selects.corrected_alg].corrected.resistance.size()));
                    }
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Measurement Periodic Reactance Data")) {
                    if(firsts.periodic.corrected_alg) {
                        ImPlot::SetupAxes("Time", "REACTANCE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.periodic.corrected_alg = false;
                    } else {
                        ImPlot::SetupAxesLabels("Time", "REACTANCE");
                    }

                    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    if(periodic_vectors.points.empty() == false) {
                        ImPlot::PlotLine("REACTANCE [Ohm]", periodic_vectors.time_points.data(), periodic_vectors.points[periodic_index_selects.corrected_alg].corrected.reactance.data(), std::min(periodic_vectors.time_points.size(), periodic_vectors.points[periodic_index_selects.corrected_alg].corrected.reactance.size()));
                    }
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_file_name[] { "measurement_periodic_corrected_alg" };
                    ns::CorrectedAlgGraph3D_File<graph_file_name, ns::Graph3D_Names::zfreq_corrected_alg, ns::Freq, double, ns::ValueNames::unix_timestamp> graph3D_file;
                    const size_t wished_size { periodic_vectors.freq.size() };
                    graph3D_file.graph_3D.points.reserve(wished_size);
                    for(size_t i = 0; i < wished_size; i++) {
                        graph3D_file.graph_3D.points.push_back(
                            {
                                periodic_vectors.freq[i],
                            }
                        );
                        const size_t min { std::min(std::min(periodic_vectors.time_points.size(), periodic_vectors.points[i].corrected.resistance.size()), periodic_vectors.points[i].corrected.reactance.size()) };
                        graph3D_file.graph_3D.points[i].graph_2D.points.reserve(min);
                        for(size_t j = 0; j < min; j++) {
                            graph3D_file.graph_3D.points[i].graph_2D.points.push_back(
                                {
                                    periodic_vectors.time_points[j],
                                    periodic_vectors.points[i].corrected.resistance[j],
                                    periodic_vectors.points[i].corrected.reactance[j],
                                }
                            );
                        }
                    }

                    ns::save_to_fs(graph3D_file);
                }
            }

            void Measurement::update_periodic_vectors(
                const std::vector<float>& freq,
                std::queue<Measure::PeriodicPoint>& periodic_points
            ) {
                if(freq != periodic_vectors.freq) {
                    periodic_vectors.freq = freq;
                    periodic_vectors.time_points.clear();
                    periodic_vectors.points.clear();
                    periodic_vectors.points.resize(periodic_vectors.freq.size());
                    const uint32_t tmp_first_freq = static_cast<uint32_t>(freq.front());
                    periodic_freq_selects = PeriodicFreqSelects {
                        tmp_first_freq,
                        tmp_first_freq,
                        tmp_first_freq,
                        tmp_first_freq,
                    };
                    periodic_index_selects = PeriodicIndexSelects { 0, 0, 0, 0 };
                }

                const auto front_point { periodic_points.front() };
                periodic_vectors.time_points.push_back(front_point.time_point);

                for(size_t i = 0; i < periodic_vectors.points.size(); i++) {
                    periodic_vectors.points[i].raw.real_data.push_back(front_point.raw_measurement[i].get_real_data());
                    periodic_vectors.points[i].raw.imag_data.push_back(front_point.raw_measurement[i].get_imag_data());
                    periodic_vectors.points[i].raw.magnitude.push_back(front_point.raw_measurement[i].get_raw_magnitude<float>());
                    periodic_vectors.points[i].raw.phase.push_back(front_point.raw_measurement[i].get_raw_phase<float>());

                    periodic_vectors.points[i].corrected.impedance.push_back(front_point.measurement[i].get_magnitude());
                    periodic_vectors.points[i].corrected.phase.push_back(front_point.measurement[i].get_phase());
                    periodic_vectors.points[i].corrected.resistance.push_back(front_point.measurement[i].get_resistance());
                    periodic_vectors.points[i].corrected.reactance.push_back(front_point.measurement[i].get_reactance());
                }

                periodic_points.pop();
            }

            void Measurement::update_single_vectors(
                const std::vector<float>& freq,
                const std::vector<AD5933::Data>& raw_measurement,
                const std::vector<AD5933::Measurement<float>>& measurement
            ) {
                single_vectors.freq = freq;

                single_vectors.raw.real_data.resize(raw_measurement.size());
                std::transform(raw_measurement.begin(), raw_measurement.end(), single_vectors.raw.real_data.begin(), [](const AD5933::Data &e) { return static_cast<float>(e.get_real_data()); });

                single_vectors.raw.imag_data.resize(raw_measurement.size());
                std::transform(raw_measurement.begin(), raw_measurement.end(), single_vectors.raw.imag_data.begin(), [](const AD5933::Data &e) { return static_cast<float>(e.get_imag_data()); });

                single_vectors.raw.magnitude.resize(raw_measurement.size());
                std::transform(raw_measurement.begin(), raw_measurement.end(), single_vectors.raw.magnitude.begin(), [](const AD5933::Data &e) { return e.get_raw_magnitude<float>(); });

                single_vectors.raw.phase.resize(raw_measurement.size());
                std::transform(raw_measurement.begin(), raw_measurement.end(), single_vectors.raw.phase.begin(), [](const AD5933::Data &e) { return e.get_raw_phase<float>(); });

                single_vectors.corrected.impedance.resize(measurement.size());
                std::transform(measurement.begin(), measurement.end(), single_vectors.corrected.impedance.begin(), [](const AD5933::Measurement<float> &e) { return e.get_magnitude(); });               

                single_vectors.corrected.phase.resize(measurement.size());
                std::transform(measurement.begin(), measurement.end(), single_vectors.corrected.phase.begin(), [](const AD5933::Measurement<float> &e) { return e.get_phase(); });

                single_vectors.corrected.resistance.resize(measurement.size());
                std::transform(measurement.begin(), measurement.end(), single_vectors.corrected.resistance.begin(), [](const AD5933::Measurement<float> &e) { return e.get_resistance(); });

                single_vectors.corrected.reactance.resize(measurement.size());
                std::transform(measurement.begin(), measurement.end(), single_vectors.corrected.reactance.begin(), [](const AD5933::Measurement<float> &e) { return e.get_reactance(); });
            }

            void Measurement::draw_single_raw_data() {
                if(ImPlot::BeginPlot("Measurement Single Raw Real Data")) {
                    if(firsts.single.raw) {
                        ImPlot::SetupAxes("f [Hz]", "REAL_DATA", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "REAL_DATA");
                    }
                    ImPlot::PlotLine("REAL_DATA [1/Ohm]", single_vectors.freq.data(), single_vectors.raw.real_data.data(), std::min(single_vectors.freq.size(), single_vectors.raw.real_data.size()));
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Measurement Single Raw Imag Data")) {
                    if(firsts.single.raw) {
                        ImPlot::SetupAxes("f [Hz]", "IMAG_DATA", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.single.raw = false;
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "IMAG_DATA");
                    }

                    ImPlot::PlotLine("IMAG_DATA [1/Ohm]", single_vectors.freq.data(), single_vectors.raw.imag_data.data(), std::min(single_vectors.freq.size(), single_vectors.raw.imag_data.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_name[] { "measurement_single_raw" };
                    ns::RawDataGraph2D_File<graph_name, float, ns::ValueNames::freq> graph_file {};
                    ns::load_graph2D_file(
                        single_vectors.freq,
                        single_vectors.raw.real_data,
                        single_vectors.raw.imag_data,
                        graph_file
                    );
                    ns::save_to_fs(graph_file);
                }
            }

            void Measurement::draw_single_calculated_data() {
                if(ImPlot::BeginPlot("Measurement Calculated Magnitude")) {
                    if(firsts.single.calculated) {
                        ImPlot::SetupAxes("f [Hz]", "RAW_MAGNITUDE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "RAW_MAGNITUDE");
                    }

                    ImPlot::PlotLine("RAW_MAGNITUDE [1/Ohm]", single_vectors.freq.data(), single_vectors.raw.magnitude.data(), std::min(single_vectors.freq.size(), single_vectors.raw.magnitude.size()));
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Measurement Calculated Phase")) {
                    if(firsts.single.calculated) {
                        ImPlot::SetupAxes("f [Hz]", "RAW_PHASE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.single.calculated = false;
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "RAW_PHASE");
                    }

                    ImPlot::PlotLine("RAW_PHASE [rad]", single_vectors.freq.data(), single_vectors.raw.phase.data(), std::min(single_vectors.freq.size(), single_vectors.raw.phase.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_name[] { "measurement_single_calculated" };
                    ns::CalculatedGraph2D_File<graph_name, float, ns::ValueNames::freq> graph_file {};
                    ns::load_graph2D_file(
                        single_vectors.freq,
                        single_vectors.raw.magnitude,
                        single_vectors.raw.phase,
                        graph_file
                    );
                    ns::save_to_fs(graph_file);
                }
            }

            void Measurement::draw_single_corrected_gon_data() {
                if(ImPlot::BeginPlot("Measurement Single Corrected Impedance")) {
                    if(firsts.single.corrected_gon) {
                        ImPlot::SetupAxes("f [Hz]", "IMPEDANCE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "IMPEDANCE");
                    }

                    ImPlot::PlotLine("IMPEDANCE [Ohm]", single_vectors.freq.data(), single_vectors.corrected.impedance.data(), std::min(single_vectors.freq.size(), single_vectors.corrected.impedance.size()));
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Measurement Single Calculated Phase")) {
                    if(firsts.single.corrected_gon) {
                        ImPlot::SetupAxes("f [Hz]", "PHASE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.single.corrected_gon = false;
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "PHASE");
                    }

                    ImPlot::PlotLine("PHASE [rad]", single_vectors.freq.data(), single_vectors.corrected.phase.data(), std::min(single_vectors.freq.size(), single_vectors.corrected.phase.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_name[] { "measurement_single_corrected_gon" };
                    ns::CorrectedGonGraph2D_File<graph_name, float, ns::ValueNames::freq> graph_file {};
                    ns::load_graph2D_file(
                        single_vectors.freq,
                        single_vectors.corrected.impedance,
                        single_vectors.corrected.phase,
                        graph_file
                    );
                    ns::save_to_fs(graph_file);
                }
            }

            void Measurement::draw_single_corrected_alg_data() {
                if(ImPlot::BeginPlot("Measurement Single Corrected Resistance")) {
                    if(firsts.single.corrected_alg) {
                        ImPlot::SetupAxes("f [Hz]", "RESISTANCE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "RESISTANCE");
                    }

                    ImPlot::PlotLine("RESISTANCE [Ohm]", single_vectors.freq.data(), single_vectors.corrected.resistance.data(), std::min(single_vectors.freq.size(), single_vectors.corrected.resistance.size()));
                    ImPlot::EndPlot();
                }
                if(ImPlot::BeginPlot("Measurement Single Corrected Reactance")) {
                    if(firsts.single.corrected_alg) {
                        ImPlot::SetupAxes("f [Hz]", "REACTANCE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        firsts.single.corrected_alg = false;
                    } else {
                        ImPlot::SetupAxesLabels("f [Hz]", "REACTANCE");
                    }

                    ImPlot::PlotLine("REACTANCE [Ohm]", single_vectors.freq.data(), single_vectors.corrected.reactance.data(), std::min(single_vectors.freq.size(), single_vectors.corrected.reactance.size()));
                    ImPlot::EndPlot();
                }

                if(ImGui::Button("Save")) {
                    static constexpr char graph_name[] { "measurement_single_corrected_alg" };
                    ns::CorrectedAlgGraph2D_File<graph_name, float, ns::ValueNames::freq> graph_file {};
                    ns::load_graph2D_file(
                        single_vectors.freq,
                        single_vectors.corrected.resistance,
                        single_vectors.corrected.reactance,
                        graph_file
                    );
                    ns::save_to_fs(graph_file);
                }
            }
        }
    }
}