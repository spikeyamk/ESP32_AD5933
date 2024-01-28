#include "imgui_internal.h"
#include "implot.h"

#include "gui/windows/plots/measurement.hpp"

namespace GUI {
   namespace Windows {
        void freq_sweep_raw_data_plots(
            std::vector<float> &frequency_vector,
            std::vector<AD5933::Data> &raw_measurement_data
        ) {
            if(ImPlot::BeginPlot("Measurement Raw Real Data")) {
                ImPlot::SetupAxes("f [Hz]", "REAL_DATA");
                std::vector<float> real_data_vector(raw_measurement_data.size());
                std::transform(raw_measurement_data.begin(), raw_measurement_data.end(), real_data_vector.begin(), [](AD5933::Data &e) { return static_cast<float>(e.get_real_data()); });
                ImPlot::PlotLine("REAL_DATA [1/Ohm]", frequency_vector.data(), real_data_vector.data(), frequency_vector.size());
                ImPlot::EndPlot();
            }
            if(ImPlot::BeginPlot("Measurement Raw Imag Data")) {
                ImPlot::SetupAxes("f [Hz]", "IMAG_DATA");
                std::vector<float> imag_data_vector(raw_measurement_data.size());
                std::transform(raw_measurement_data.begin(), raw_measurement_data.end(), imag_data_vector.begin(), [](AD5933::Data &e) { return static_cast<float>(e.get_imag_data()); });
                ImPlot::PlotLine("IMAG_DATA [1/Ohm]", frequency_vector.data(), imag_data_vector.data(), frequency_vector.size());
                ImPlot::EndPlot();
            }
        }

        void freq_sweep_calculated_data_plots(
            std::vector<float> &frequency_vector,
            std::vector<AD5933::Data> &raw_measurement_data
        ) {
            if(ImPlot::BeginPlot("Measurement Calculated Magnitude")) {
                ImPlot::SetupAxes("f [Hz]", "RAW_MAGNITUDE");
                std::vector<float> raw_magnitude_vector(raw_measurement_data.size());
                std::transform(raw_measurement_data.begin(), raw_measurement_data.end(), raw_magnitude_vector.begin(), [](AD5933::Data &e) { return e.get_raw_magnitude<float>(); });
                ImPlot::PlotLine("RAW_MAGNITUDE [1/Ohm]", frequency_vector.data(), raw_magnitude_vector.data(), frequency_vector.size());
                ImPlot::EndPlot();
            }
            if(ImPlot::BeginPlot("Measurement Calculated Phase")) {
                ImPlot::SetupAxes("f [Hz]", "RAW_PHASE");
                std::vector<float> raw_phase_vector(raw_measurement_data.size());
                std::transform(raw_measurement_data.begin(), raw_measurement_data.end(), raw_phase_vector.begin(), [](AD5933::Data &e) { return e.get_raw_phase<float>(); });
                ImPlot::PlotLine("RAW_PHASE [rad]", frequency_vector.data(), raw_phase_vector.data(), frequency_vector.size());
                ImPlot::EndPlot();
            }
        }

        void freq_sweep_corrected_data_plots(
            std::vector<float> &frequency_vector,
            std::vector<AD5933::Measurement<float>> &measurement_data
        ) {
            if(ImPlot::BeginPlot("Measurement Corrected Data")) {
                ImPlot::SetupAxes("f [Hz]", "CORRECTED_IMPEDANCE");
                std::vector<float> corrected_impedance_vector(measurement_data.size());
                std::transform(measurement_data.begin(), measurement_data.end(), corrected_impedance_vector.begin(), [](AD5933::Measurement<float> &e) {
                    return e.get_magnitude();
                });
                ImPlot::PlotLine("CORRECTED_IMPEDANCE [Ohm]", frequency_vector.data(), corrected_impedance_vector.data(), frequency_vector.size());
                ImPlot::EndPlot();
            }
            if(ImPlot::BeginPlot("Measurement Calculated Phase")) {
                ImPlot::SetupAxes("f [Hz]", "CORRECTED_PHASE");
                std::vector<float> corrected_phase_vector(measurement_data.size());
                std::transform(measurement_data.begin(), measurement_data.end(), corrected_phase_vector.begin(), [](AD5933::Measurement<float> &e) {
                    return e.get_phase();
                });
                ImPlot::PlotLine("CORRECTED_PHASE [rad]", frequency_vector.data(), corrected_phase_vector.data(), frequency_vector.size());
                ImPlot::EndPlot();
            }
        }

        void freq_sweep_impedance_data_plots(
            std::vector<float> &frequency_vector,
            std::vector<AD5933::Measurement<float>> &measurement_data
        ) {
            if(ImPlot::BeginPlot("Measurement Resistance Data")) {
                ImPlot::SetupAxes("f [Hz]", "RESISTANCE");
                std::vector<float> resistance_vector(measurement_data.size());
                std::transform(measurement_data.begin(), measurement_data.end(), resistance_vector.begin(), [](AD5933::Measurement<float> &e) {
                    return e.get_resistance();
                });
                ImPlot::PlotLine("RESISTANCE [Ohm]", frequency_vector.data(), resistance_vector.data(), frequency_vector.size());
                ImPlot::EndPlot();
            }
            if(ImPlot::BeginPlot("Measurement Reactance Data")) {
                ImPlot::SetupAxes("f [Hz]", "REACTANCE");
                std::vector<float> reactance_vector(measurement_data.size());
                std::transform(measurement_data.begin(), measurement_data.end(), reactance_vector.begin(), [](AD5933::Measurement<float> &e) {
                    return e.get_reactance();
                });
                ImPlot::PlotLine("REACTANCE [Ohm]", frequency_vector.data(), reactance_vector.data(), frequency_vector.size());
                ImPlot::EndPlot();
            }
        }
    }

    namespace Windows {
        void measurement_plots(ImGuiID side_id, bool &enable, Client& client) {
            std::string name = "Measurement Plots##" + std::to_string(client.index);
            static size_t first = 0;
            if(first == client.index) {
                ImGui::DockBuilderDockWindow(name.c_str(), side_id);
            }

            ImPlot::CreateContext();
            if(ImGui::Begin(name.c_str(), &enable) == false) {
                ImGui::End();
                return;
            }

            if(first == client.index) {
                ImGui::End();
                first++;
                return;
            }

            const auto start_freq = client.configure_captures.config.get_start_freq();
            const auto inc_freq = client.configure_captures.config.get_inc_freq();
            std::vector<float> frequency_vector(client.measurement.size());
            std::generate(
                frequency_vector.begin(),
                frequency_vector.end(),
                [start_freq, inc_freq, n = 0.0] () mutable {
                    return static_cast<float>(start_freq.unwrap() + ((n++) * inc_freq.unwrap()));
                }
            );

            if(ImGui::BeginTabBar("FREQ_SWEEP_BAR")) {
                if (ImGui::BeginTabItem("RAW_DATA")) {
                    freq_sweep_raw_data_plots(frequency_vector, client.raw_measurement);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("CALCULATED_DATA")) {
                    freq_sweep_calculated_data_plots(frequency_vector, client.raw_measurement);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("CORRECTED_DATA")) {
                    freq_sweep_corrected_data_plots(frequency_vector, client.measurement);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("IMPEDANCE_DATA")) {
                    freq_sweep_impedance_data_plots(frequency_vector, client.measurement);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::End();
        }
    }
}
