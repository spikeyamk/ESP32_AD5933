#include "imgui_internal.h"
#include "implot.h"

#include "gui/windows/plots/calibration.hpp"

namespace GUI {
    namespace Windows {
        void calibration_plots(int i, ImGuiID side_id, bool &enable, Client &client) {
            std::string name = "Calibration Plots##" + std::to_string(i);
            static int first = 0;
            if(first == i) {
                ImGui::DockBuilderDockWindow(name.c_str(), side_id);
            }

            ImPlot::CreateContext();
            if(ImGui::Begin(name.c_str(), &enable) == false) {
                ImGui::End();
                return;
            }

            if(first == i) {
                ImGui::End();
                first++;
                return;
            }

            const auto start_freq = client.configure_captures.config.get_start_freq();
            const auto inc_freq = client.configure_captures.config.get_inc_freq();
            std::vector<float> frequency_vector(client.calibration.size());
            std::generate(
                frequency_vector.begin(),
                frequency_vector.end(),
                [start_freq, inc_freq, n = 0.0f] () mutable {
                    return static_cast<float>(start_freq.unwrap() + ((n++) * inc_freq.unwrap()));
                }
            );

            if(ImGui::BeginTabBar("Single_Plots")) {
                if(ImGui::BeginTabItem("Single")) {
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabBar("Calibration_PlotsBar")) {
                    if(ImGui::BeginTabItem("RAW_DATA")) {
                        std::vector<float> real_data_vector;
                        real_data_vector.reserve(client.raw_calibration.size());
                        std::transform(client.raw_calibration.begin(), client.raw_calibration.end(), real_data_vector.begin(), [](AD5933::Data &e) { return static_cast<float>(e.get_real_data()); });
                        if(ImPlot::BeginPlot("Calibration Raw Real Data")) {
                            ImPlot::SetupAxes("f [Hz]", "REAL_DATA");
                            ImPlot::PlotLine("REAL_DATA [1/Ohm]", frequency_vector.data(), real_data_vector.data(), frequency_vector.size());
                            ImPlot::EndPlot();
                        }

                        std::vector<float> imag_data_vector;
                        imag_data_vector.reserve(client.raw_calibration.size());
                        std::transform(client.raw_calibration.begin(), client.raw_calibration.end(), imag_data_vector.begin(), [](AD5933::Data &e) { return static_cast<float>(e.get_imag_data()); });
                        if(ImPlot::BeginPlot("Calibration Raw Imag Data")) {
                            ImPlot::SetupAxes("f [Hz]", "IMAG_DATA");
                            ImPlot::PlotLine("IMAG_DATA [1/Ohm]", frequency_vector.data(), imag_data_vector.data(), frequency_vector.size());
                            ImPlot::EndPlot();
                        }

                        if(ImGui::BeginTable("RawData", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
                            ImGui::TableSetupColumn("Frequency [Hz]");
                            ImGui::TableSetupColumn("Real [1/Ohm]");
                            ImGui::TableSetupColumn("Imag [1/Ohm]");
                            ImGui::TableHeadersRow();
                            std::for_each(frequency_vector.begin(), frequency_vector.end(), [index = 0,  &real_data_vector, &imag_data_vector](const float &f) mutable {
                                ImGui::TableNextRow();
                                if(index == 0) {
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                    ImGui::TableSetColumnIndex(2);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                }

                                ImGui::TableNextColumn();
                                std::string frequency_string { std::to_string(f) };
                                ImGui::PushID(index);
                                ImGui::InputText("##Frequency", frequency_string.data(), frequency_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::TableNextColumn();
                                std::string real_string { std::to_string(real_data_vector[index]) };
                                ImGui::InputText("##RealData", real_string.data(), real_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::TableNextColumn();
                                std::string imag_string { std::to_string(imag_data_vector[index]) };
                                ImGui::InputText("##ImagData", imag_string.data(), imag_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::PopID();
                                index++;
                            });
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("CALCULATED_DATA")) {
                        std::vector<float> raw_magnitude_vector;
                        raw_magnitude_vector.reserve(client.raw_calibration.size());
                        std::transform(client.raw_calibration.begin(), client.raw_calibration.end(), raw_magnitude_vector.begin(), [](AD5933::Data &e) { return e.get_raw_magnitude<float>(); });
                        if(ImPlot::BeginPlot("Calibration Calculated Magnitude")) {
                            ImPlot::SetupAxes("f [Hz]", "RAW_MAGNITUDE");
                            ImPlot::PlotLine("RAW_MAGNITUDE [1/Ohm]", frequency_vector.data(), raw_magnitude_vector.data(), frequency_vector.size());
                            ImPlot::EndPlot();
                        }

                        std::vector<float> raw_phase_vector;
                        raw_phase_vector.reserve(client.raw_calibration.size());
                        std::transform(client.raw_calibration.begin(), client.raw_calibration.end(), raw_phase_vector.begin(), [](AD5933::Data &e) { return e.get_raw_phase<float>(); });
                        if(ImPlot::BeginPlot("Calibration Calculated Phase")) {
                            ImPlot::SetupAxes("f [Hz]", "RAW_PHASE");
                            ImPlot::PlotLine("RAW_PHASE [rad]", frequency_vector.data(), raw_phase_vector.data(), frequency_vector.size());
                            ImPlot::EndPlot();
                        }

                        if(ImGui::BeginTable("RawData", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
                            ImGui::TableSetupColumn("Frequency [Hz]");
                            ImGui::TableSetupColumn("Raw Magnitude [1/Ohm]");
                            ImGui::TableSetupColumn("Raw Phase [rad]");
                            ImGui::TableHeadersRow();
                            std::for_each(frequency_vector.begin(), frequency_vector.end(), [index = 0,  &raw_magnitude_vector, &raw_phase_vector](const float &f) mutable {
                                ImGui::TableNextRow();
                                if(index == 0) {
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                    ImGui::TableSetColumnIndex(2);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                }

                                ImGui::TableNextColumn();
                                std::string frequency_string { std::to_string(f) };
                                ImGui::PushID(index);
                                ImGui::InputText("##Frequency", frequency_string.data(), frequency_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::TableNextColumn();
                                std::string real_string { std::to_string(raw_magnitude_vector[index]) };
                                ImGui::InputText("##RealData", real_string.data(), real_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::TableNextColumn();
                                std::string imag_string { std::to_string(raw_phase_vector[index]) };
                                ImGui::InputText("##ImagData", imag_string.data(), imag_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::PopID();
                                index++;
                            });
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("CALIBRATION_DATA")) {
                        std::vector<float> gain_factor_vector;
                        gain_factor_vector.reserve(client.calibration.size());
                        std::transform(
                            client.calibration.begin(),
                            client.calibration.end(),
                            gain_factor_vector.begin(),
                            [](AD5933::Calibration<float> &e) {
                                return static_cast<float>(e.get_gain_factor());
                            }
                        );
                        if(ImPlot::BeginPlot("Calibration Gain Factor")) {
                            ImPlot::SetupAxes("f [Hz]", "GAIN_FACTOR");
                            ImPlot::PlotLine("GAIN_FACTOR", frequency_vector.data(), gain_factor_vector.data(), frequency_vector.size());
                            ImPlot::EndPlot();
                        }

                        std::vector<float> system_phase_vector;
                        system_phase_vector.reserve(client.calibration.size());
                        std::transform(
                            client.calibration.begin(),
                            client.calibration.end(),
                            system_phase_vector.begin(),
                            [](AD5933::Calibration<float> &e) {
                                return static_cast<float>(e.get_system_phase()); 
                            }
                        );
                        if(ImPlot::BeginPlot("Calibration System Phase")) {
                            ImPlot::SetupAxes("f [Hz]", "SYSTEM_PHASE");
                            ImPlot::PlotLine("SYSTEM_PHASE [rad]", frequency_vector.data(), system_phase_vector.data(), frequency_vector.size());
                            ImPlot::EndPlot();
                        }

                        if(ImGui::BeginTable("RawData", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
                            ImGui::TableSetupColumn("Frequency [Hz]");
                            ImGui::TableSetupColumn("Gain Factor");
                            ImGui::TableSetupColumn("System Phase [rad]");
                            ImGui::TableHeadersRow();
                            std::for_each(frequency_vector.begin(), frequency_vector.end(), [index = 0,  &gain_factor_vector, &system_phase_vector](const float &f) mutable {
                                ImGui::TableNextRow();
                                if(index == 0) {
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                    ImGui::TableSetColumnIndex(2);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                }

                                ImGui::TableNextColumn();
                                std::string frequency_string { std::to_string(f) };
                                ImGui::PushID(index);
                                ImGui::InputText("##Frequency", frequency_string.data(), frequency_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::TableNextColumn();
                                std::string gain_factor_string { std::to_string(gain_factor_vector[index]) };
                                ImGui::InputText("##GainFactor", gain_factor_string.data(), gain_factor_vector.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::TableNextColumn();
                                std::string system_phase_string { std::to_string(system_phase_vector[index]) };
                                ImGui::InputText("##SystemPhase", system_phase_string.data(), system_phase_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::PopID();
                                index++;
                            });
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabBar();
            }

            ImGui::End();
        }
    }
}