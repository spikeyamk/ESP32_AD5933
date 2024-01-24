#include <iostream>
#include <sstream>
#include <chrono>
#include <functional>
#include <tuple>
#include <memory>
#include <optional>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <vector>
#include <numeric>
#include <list>
#include <type_traits>

#include <trielo/trielo.hpp>

#include "implot.h"
#include "magic/packets.hpp"
#include "imgui_console/imgui_console.h"

#include "gui/imgui_sdl.hpp"
#include "ble_client/ble_client.hpp"
#include "gui/boilerplate.hpp"
#include "gui/spinner.hpp"

#include "ad5933/masks/maps.hpp"
#include "ad5933/config/config.hpp"
#include "ad5933/calibration/calibration.hpp"
#include "ad5933/debug_data/debug_data.hpp"
#include "gui/windows/captures.hpp"
#include "gui/windows/connecting.hpp"
#include "ad5933/measurement/measurement.hpp"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

namespace GUI {
    // Related to dbg_registers_window   
    bool send_debug_start_req_thread_cb(std::optional<ESP32_AD5933> &esp32_ad5933) {
        return esp32_ad5933.value().send(std::string(Magic::Packets::Debug::start.begin(), Magic::Packets::Debug::start.end()));
    }

    bool send_debug_end_req_thread_cb(std::optional<ESP32_AD5933> &esp32_ad5933) {
        return esp32_ad5933.value().send(std::string(Magic::Packets::Debug::end.begin(), Magic::Packets::Debug::end.end()));
    }
   
    void send_dump_all_registers_req_thread_cb(
        std::optional<ESP32_AD5933> &esp32_ad5933,
        std::atomic<std::shared_ptr<Windows::Captures::HexDebugReadWriteRegisters >> &hex_dbg_rw_captures
    ) {
        esp32_ad5933.value().send(std::string(Magic::Packets::Debug::dump_all_registers.begin(), Magic::Packets::Debug::dump_all_registers.end()));
        //const auto rx_register_info = esp32_ad5933.value().rx_payload.read();
        //esp32_ad5933.value().rx_payload.clean();
        //hex_dbg_rw_captures.load()->update_captures(rx_register_info);
    }

    void dbg_registers_thread_cb(std::atomic<int> &debug_window_enable) {
        while(debug_window_enable.load() > 0) {
            debug_window_enable.wait(debug_window_enable);
            if(debug_window_enable.load() == 2) {
                std::cout << "GUI: Loading rx_payload content\n";
                debug_window_enable.store(1);
            }
        }

        debug_window_enable.store(0);
        std::cout << "GUI: ending dbg_registers_thread_cb\n";
    }

    void program_all_registers_thread_cb(std::array<uint8_t, 12> raw_message, std::optional<ESP32_AD5933> &esp32_ad5933) {
        auto buf_to_send = Magic::Packets::Debug::program_all_registers;
        std::copy(raw_message.begin(), raw_message.end(), buf_to_send.begin());
        const auto tmp_dbg_var = std::string(buf_to_send.begin(), buf_to_send.end());
        esp32_ad5933.value().send(tmp_dbg_var);
    }

    void dbg_program_all_registers_thread_cb(
        std::array<uint8_t, 12> raw_message,
        std::optional<ESP32_AD5933> &esp32_ad5933,
        std::atomic<std::shared_ptr<Windows::Captures::HexDebugReadWriteRegisters>> &hex_dbg_rw_captures
    ) {
        program_all_registers_thread_cb(raw_message, esp32_ad5933);

        std::thread send_dump_all_registers_thread(
            send_dump_all_registers_req_thread_cb,
            std::ref(esp32_ad5933),
            std::ref(hex_dbg_rw_captures)
        );
        send_dump_all_registers_thread.join();
    }

    void control_command_thread_cb(
        const AD5933::Masks::Or::Ctrl::HB::Command command,
        std::optional<ESP32_AD5933> &esp32_ad5933,
        std::atomic<std::shared_ptr<Windows::Captures::HexDebugReadWriteRegisters >> &hex_dbg_rw_captures
    ) {
        auto buf_to_send = Magic::Packets::Debug::control_HB_command;
        buf_to_send[0] = static_cast<uint8_t>(command);
        esp32_ad5933.value().send(std::string(buf_to_send.begin(), buf_to_send.end()));
        send_dump_all_registers_req_thread_cb(esp32_ad5933, hex_dbg_rw_captures);
    }

    void configure_cb(
        std::array<uint8_t, 12> raw_array,
        std::optional<ESP32_AD5933> &esp32_ad5933,
        bool &configured
    ) {
        auto buf_to_send = Magic::Packets::FrequencySweep::configure;
        std::copy(raw_array.begin(), raw_array.end(), buf_to_send.begin());
        esp32_ad5933.value().send(std::string(buf_to_send.begin(), buf_to_send.end()));
        configured = true;
    }

    void freq_sweep_end_cb(std::optional<ESP32_AD5933> &esp32_ad5933, bool &configured) {
        auto buf_to_send = Magic::Packets::FrequencySweep::end;
        esp32_ad5933.value().send(std::string(buf_to_send.begin(), buf_to_send.end()));
        configured = false;
    }

    void debug_registers_window(
        std::optional<ESP32_AD5933> &esp32_ad5933,
        std::atomic<std::shared_ptr<Windows::Captures::HexDebugReadWriteRegisters>> &hex_dbg_rw_captures
    ) {
        //static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
        ImGui::Begin("Register Debug", NULL, 0);                          // Create a window called "Hello, world!" and append into it.
        auto tmp_hex_dbg_rw_captures = *(hex_dbg_rw_captures.load());

        static bool debug_started = false;
        if(debug_started == false && ImGui::Button("Debug Start")) {
            debug_started = true;
            std::thread (send_debug_start_req_thread_cb, std::ref(esp32_ad5933)).detach();
        }

        if(debug_started == true) {
            ImGui::SameLine();
            if(ImGui::Button("Dump All Registers")) {
                std::thread(
                    send_dump_all_registers_req_thread_cb,
                    std::ref(esp32_ad5933),
                    std::ref(hex_dbg_rw_captures)
                ).detach();
            }; ImGui::SameLine();

            if(ImGui::Button("Program Read/Write Registers")) {
                std::thread(
                    dbg_program_all_registers_thread_cb,
                    tmp_hex_dbg_rw_captures.config.to_raw_array(),
                    std::ref(esp32_ad5933),
                    std::ref(hex_dbg_rw_captures)
                ).detach();
            }; ImGui::SameLine();

            if(ImGui::Button("Debug End")) {
                std::thread(send_debug_end_req_thread_cb, std::ref(esp32_ad5933)).detach();
                debug_started = false;
            }
        }

        ImGui::Separator();

        ImGui::Text("Decoded Register Values");
        ImGui::Separator();

        ImGui::Text("ControlHB:");
        ImGui::Text("\tControlHB Command: %s", AD5933::Masks::get_map_str(tmp_hex_dbg_rw_captures.config.get_command(), AD5933::Masks::Or::Ctrl::HB::command_map));
        ImGui::Text("\tExcitation Output Voltage Range: %s", AD5933::Masks::get_map_str(tmp_hex_dbg_rw_captures.config.get_voltage_range(), AD5933::Masks::Or::Ctrl::HB::voltage_map));
        ImGui::Text("\tPGA Gain: %s", AD5933::Masks::get_map_str(tmp_hex_dbg_rw_captures.config.get_PGA_gain(), AD5933::Masks::Or::Ctrl::HB::pga_gain_map));
        ImGui::Separator();

        ImGui::Text("ControlLB:");
        ImGui::Text("\tSystem Clock Source: %s", AD5933::Masks::get_map_str(tmp_hex_dbg_rw_captures.config.get_sysclk_src(), AD5933::Masks::Or::Ctrl::LB::sysclk_src_map)); 
        ImGui::Separator();

        ImGui::Text("Start Frequency: %f", tmp_hex_dbg_rw_captures.config.get_start_freq().to_float());
        ImGui::Text("Frequency Increment: %f", tmp_hex_dbg_rw_captures.config.get_inc_freq().to_float());
        ImGui::Text("Number of Increments: %u", tmp_hex_dbg_rw_captures.config.get_num_of_inc().unwrap());
        ImGui::Text("Number of Settling Time Cycles: %u", tmp_hex_dbg_rw_captures.config.get_settling_time_cycles_number().unwrap());
        ImGui::Text("Settling Time Cycles Multiplier: %s", AD5933::Masks::get_map_str(tmp_hex_dbg_rw_captures.config.get_settling_time_cycles_multiplier(), AD5933::Masks::Or::SettlingTimeCyclesHB::multiplier_map));
        ImGui::Separator();
        ImGui::Text("Status: %s", AD5933::Masks::get_map_str(tmp_hex_dbg_rw_captures.data.get_status(), AD5933::Masks::Or::status_map));
        ImGui::Separator();

        ImGui::Text("Temperature: %f °C", tmp_hex_dbg_rw_captures.data.get_temperature());

        ImGui::InputText("Real Part",
            std::to_string(tmp_hex_dbg_rw_captures.data.get_real_part()).data(),
            std::to_string(tmp_hex_dbg_rw_captures.data.get_real_part()).size(),
            ImGuiInputTextFlags_ReadOnly
        );

        ImGui::InputText("Imaginary Part",
            std::to_string(tmp_hex_dbg_rw_captures.data.get_imag_part()).data(),
            std::to_string(tmp_hex_dbg_rw_captures.data.get_imag_part()).size(),
            ImGuiInputTextFlags_ReadOnly
        );

        ImGui::InputText("Raw Magnitude",
            std::to_string(tmp_hex_dbg_rw_captures.data.get_raw_magnitude()).data(),
            std::to_string(tmp_hex_dbg_rw_captures.data.get_raw_magnitude()).size(),
            ImGuiInputTextFlags_ReadOnly
        );

        static const auto float_to_str_lambda = [](double number) -> std::string {
            std::ostringstream stream;

            // Calculate the magnitude of the number
            double magnitude = std::abs(number);
            int precision = (magnitude == 0.0f) ? 0 : -static_cast<int>(std::floor(std::log10(magnitude))) + 7;

            stream << std::setprecision(precision) << number;

            return stream.str();
        };

        ImGui::InputText("Raw Phase [rad]",
            float_to_str_lambda(tmp_hex_dbg_rw_captures.data.get_raw_phase()).data(),
            float_to_str_lambda(tmp_hex_dbg_rw_captures.data.get_raw_phase()).size(),
            ImGuiInputTextFlags_ReadOnly
        );

        ImGui::Separator();

        ImGui::Text("Read/Write Registers");
        {
            ImGui::Separator();
            ImGui::InputText("CTRL_HB (0x80)", tmp_hex_dbg_rw_captures.ctrl_HB.data(), tmp_hex_dbg_rw_captures.ctrl_HB.size());
            ImGui::InputText("CTRL_LB (0x81)", tmp_hex_dbg_rw_captures.ctrl_LB.data(), tmp_hex_dbg_rw_captures.ctrl_LB.size());
            ImGui::Separator();
            ImGui::InputText("FREQ_START_HB (0x82)", tmp_hex_dbg_rw_captures.freq_start_HB.data(), tmp_hex_dbg_rw_captures.freq_start_HB.size());
            ImGui::InputText("FREQ_START_MB (0x83)", tmp_hex_dbg_rw_captures.freq_start_MB.data(), tmp_hex_dbg_rw_captures.freq_start_MB.size());
            ImGui::InputText("FREQ_START_LB (0x84)", tmp_hex_dbg_rw_captures.freq_start_LB.data(), tmp_hex_dbg_rw_captures.freq_start_LB.size());
            ImGui::Separator();
            ImGui::InputText("FREQ_INC_HB (0x85)", tmp_hex_dbg_rw_captures.freq_inc_HB.data(), tmp_hex_dbg_rw_captures.freq_inc_HB.size());
            ImGui::InputText("FREQ_INC_MB (0x86)", tmp_hex_dbg_rw_captures.freq_inc_MB.data(), tmp_hex_dbg_rw_captures.freq_inc_MB.size());
            ImGui::InputText("FREQ_INC_LB (0x87)", tmp_hex_dbg_rw_captures.freq_inc_LB.data(), tmp_hex_dbg_rw_captures.freq_inc_LB.size());
            ImGui::Separator();
            ImGui::InputText("NUM_OF_INC_HB (0x88)", tmp_hex_dbg_rw_captures.num_of_inc_HB.data(), tmp_hex_dbg_rw_captures.num_of_inc_HB.size());
            ImGui::InputText("NUM_OF_INC_LB (0x89)", tmp_hex_dbg_rw_captures.num_of_inc_LB.data(), tmp_hex_dbg_rw_captures.num_of_inc_LB.size());
            ImGui::Separator();
            ImGui::InputText("NUM_OF_SETTLING_TIME_CYCLES_HB (0x8A)", tmp_hex_dbg_rw_captures.settling_time_cycles_HB.data(), tmp_hex_dbg_rw_captures.settling_time_cycles_HB.size());
            ImGui::InputText("NUM_OF_SETTLING_TIME_CYCLES_LB (0x8B)", tmp_hex_dbg_rw_captures.settling_time_cycles_LB.data(), tmp_hex_dbg_rw_captures.settling_time_cycles_LB.size());

            ImGui::Separator();

            {
            ImGui::Text("Send Control Register Command Controls");
            if(ImGui::Button("Power-down mode")) {
                std::thread(control_command_thread_cb, AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode, std::ref(esp32_ad5933), std::ref(hex_dbg_rw_captures)).detach();
            } ImGui::SameLine();
                if(ImGui::Button("Standby mode")) {
                    std::thread(control_command_thread_cb, AD5933::Masks::Or::Ctrl::HB::Command::StandbyMode, std::ref(esp32_ad5933), std::ref(hex_dbg_rw_captures)).detach();
                } ImGui::SameLine();
                if(ImGui::Button("No operation (NOP_0)")) {
                    std::thread(control_command_thread_cb, AD5933::Masks::Or::Ctrl::HB::Command::Nop_0, std::ref(esp32_ad5933), std::ref(hex_dbg_rw_captures)).detach();
                }

            if(ImGui::Button("Measure temperature")) {
                std::thread(control_command_thread_cb, AD5933::Masks::Or::Ctrl::HB::Command::MeasureTemp, std::ref(esp32_ad5933), std::ref(hex_dbg_rw_captures)).detach();
            }

            if(ImGui::Button("Initialize with start frequency")) {
                std::thread(control_command_thread_cb, AD5933::Masks::Or::Ctrl::HB::Command::InitStartFreq, std::ref(esp32_ad5933), std::ref(hex_dbg_rw_captures)).detach();
            } ImGui::SameLine();
                if(ImGui::Button("Start frequency sweep")) {
                    std::thread(control_command_thread_cb, AD5933::Masks::Or::Ctrl::HB::Command::StartFreqSweep, std::ref(esp32_ad5933), std::ref(hex_dbg_rw_captures)).detach();
                } ImGui::SameLine();
                if(ImGui::Button("Increment frequency")) {
                    std::thread(control_command_thread_cb, AD5933::Masks::Or::Ctrl::HB::Command::IncFreq, std::ref(esp32_ad5933), std::ref(hex_dbg_rw_captures)).detach();
                } ImGui::SameLine();
                if(ImGui::Button("Repeat frequency")) {
                    std::thread(control_command_thread_cb, AD5933::Masks::Or::Ctrl::HB::Command::RepeatFreq, std::ref(esp32_ad5933), std::ref(hex_dbg_rw_captures)).detach();
                }
            }

            ImGui::Separator();

            ImGui::Text("Special Status Register");
            ImGui::InputText("STATUS (0x8F)", tmp_hex_dbg_rw_captures.status_capture.data(), tmp_hex_dbg_rw_captures.status_capture.size(), ImGuiInputTextFlags_ReadOnly);

            ImGui::Separator();

            ImGui::Text("Read-only Data Registers");
            ImGui::InputText("TEMP_DATA_HB (0x92)", tmp_hex_dbg_rw_captures.temp_data_HB.data(), tmp_hex_dbg_rw_captures.temp_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::InputText("TEMP_DATA_LB (0x93)", tmp_hex_dbg_rw_captures.temp_data_LB.data(), tmp_hex_dbg_rw_captures.temp_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::Separator();
            ImGui::InputText("REAL_DATA_HB (0x94)", tmp_hex_dbg_rw_captures.real_data_HB.data(), tmp_hex_dbg_rw_captures.real_data_HB.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::InputText("REAL_DATA_LB (0x95)", tmp_hex_dbg_rw_captures.real_data_LB.data(), tmp_hex_dbg_rw_captures.real_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::Separator();
            ImGui::InputText("IMAG_DATA_HB (0x96)", tmp_hex_dbg_rw_captures.imag_data_HB.data(), tmp_hex_dbg_rw_captures.imag_data_HB.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::InputText("IMAG_DATA_LB (0x97)", tmp_hex_dbg_rw_captures.imag_data_LB.data(), tmp_hex_dbg_rw_captures.imag_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::Separator();
        }

        tmp_hex_dbg_rw_captures.update_config();
        hex_dbg_rw_captures.store(std::make_shared<Windows::Captures::HexDebugReadWriteRegisters>(tmp_hex_dbg_rw_captures));
        ImGui::End();
    }
}

namespace GUI {
    void calibrate_cb(
        AD5933::uint9_t num_of_inc, 
        float calibration_impedance,
        bool &calibrating,
        bool &calibrated,
        std::atomic<float> &progress_bar_fraction,
        std::optional<ESP32_AD5933> &esp32_ad5933,
        std::vector<AD5933::Calibration<float>> &another_calibration_data,
        std::vector<AD5933::Data> &raw_calibration_data
    ) {
        esp32_ad5933.value().send(std::string(
            Magic::Packets::FrequencySweep::run.begin(),
            Magic::Packets::FrequencySweep::run.end()
        ));

        const uint16_t wished_size = num_of_inc.unwrap() + 1;
        const float progress_bar_step = 1.0f / static_cast<float>(wished_size);

        raw_calibration_data.clear();
        raw_calibration_data.reserve(wished_size);
        another_calibration_data.clear();
        another_calibration_data.reserve(wished_size);

        /*
        do {
            const auto rx_payload = esp32_ad5933.value().rx_payload.read();
            esp32_ad5933.value().rx_payload.clean();

            std::array<uint8_t, 4> raw_data;
            std::copy(rx_payload.begin(), rx_payload.begin() + 4, raw_data.begin());
            AD5933::Data tmp_data {
                raw_data
            };
            raw_calibration_data.push_back(tmp_data);

            AD5933::Calibration<float> tmp_calibration {
                tmp_data,
                calibration_impedance
            };
            another_calibration_data.push_back(tmp_calibration);

            progress_bar_fraction.fetch_add(progress_bar_step);
        } while(another_calibration_data.size() != wished_size);
        */

        calibrating = false;
        calibrated = true;
    }

    void start_sweep_cb(
        AD5933::uint9_t num_of_inc,
        bool &sweeping,
        bool &sweeped,
        std::atomic<float> &progress_bar_fraction,
        std::optional<ESP32_AD5933> &esp32_ad5933,
        bool &periodically_sweeping,
        std::vector<AD5933::Calibration<float>> &calibration_data,
        std::vector<AD5933::Data> &raw_measurement_data,
        std::vector<AD5933::Measurement<float>> &measurement_data
    ) {
        const uint16_t wished_size = num_of_inc.unwrap() + 1;
        assert(wished_size <= calibration_data.size());
        const float progress_bar_step = 1.0f / static_cast<float>(wished_size);
        do {
            const auto start = std::chrono::high_resolution_clock::now();
            esp32_ad5933.value().send(std::string(
                Magic::Packets::FrequencySweep::run.begin(),
                Magic::Packets::FrequencySweep::run.end()
            ));

            std::vector<AD5933::Data> tmp_raw_measurement_data;
            std::vector<AD5933::Measurement<float>> tmp_measurement_data;

            tmp_raw_measurement_data.reserve(wished_size);
            tmp_measurement_data.reserve(wished_size);

            progress_bar_fraction.store(0.0f);
            /*
            do {
                const auto rx_payload = esp32_ad5933.value().rx_payload.read();
                esp32_ad5933.value().rx_payload.clean();
                std::array<uint8_t, 4> raw_array;
                std::copy(rx_payload.begin(), rx_payload.begin() + 4, raw_array.begin());
                AD5933::Data tmp_data { raw_array };
                tmp_raw_measurement_data.push_back(tmp_data);
                AD5933::Measurement<float> tmp_measurement { tmp_data, calibration_data[tmp_measurement_data.size()] };
                tmp_measurement_data.push_back(tmp_measurement);

                progress_bar_fraction.fetch_add(progress_bar_step);
            } while(tmp_measurement_data.size() != wished_size);
            */

            raw_measurement_data.clear();
            raw_measurement_data = tmp_raw_measurement_data;
            measurement_data.clear();
            measurement_data = tmp_measurement_data;

            sweeped = true;
            const auto finish = std::chrono::high_resolution_clock::now();
            std::cout << "start_sweep_cb: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start) << '\n';
        } while(periodically_sweeping);

        sweeping = false;
    }
}

namespace GUI {
    inline void measurement_window(
        std::atomic<std::shared_ptr<Windows::Captures::Measurement>> &measure_captures,
        ImVec2 &measurement_window_size,
        std::optional<ESP32_AD5933> &esp32_ad5933,
        bool &plot_calibration_window_enable,
        bool &plot_freq_sweep_window_enable,
        std::vector<AD5933::Calibration<float>> &another_calibration_data,
        std::vector<AD5933::Data> &raw_calibration_data,
        std::atomic<int> &debug_window_enable,
        std::vector<AD5933::Data> &raw_measurement_data,
        std::vector<AD5933::Measurement<float>> &another_measurement_data
    ) {
        ImGui::Begin("Measurement", NULL, 0);                          // Create a window called "Hello, world!" and append into it.

        auto tmp_measure_captures = *(measure_captures.load());

        static bool calibrating = false;
        static bool calibrated = false;
        static bool sweeping = false;
        static bool sweeped = false;

        const ImGuiInputTextFlags input_flags = (calibrating == true || sweeping == true) ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None;
        ImGui::Separator(); 
        ImGui::Text("Sweep Parameters");

        ImGui::Separator();

        ImGui::InputText(
            "Start Frequency",
            tmp_measure_captures.freq_start.data(),
            tmp_measure_captures.freq_start.size(),
            input_flags
        );
        ImGui::InputText(
            "Increment Frequency",
            tmp_measure_captures.freq_inc.data(),
            tmp_measure_captures.freq_inc.size(),
            input_flags
        );
        ImGui::InputText(
            "Number of Increments",
            tmp_measure_captures.num_of_inc.data(),
            tmp_measure_captures.num_of_inc.size(),
            input_flags
        );

        ImGui::Separator();

        ImGui::InputFloat(
            "Calibration impedance",
            &tmp_measure_captures.calibration_impedance,
            0.0f,
            0.0f,
            "%f",
            input_flags
        );

        ImGui::Separator(); 

        ImGui::Combo(
            "Output Excitation Voltage Range",
            &tmp_measure_captures.voltage_range_combo,
            "2 Vppk\0""1 Vppk\0""400 mVppk\0""200 mVppk\0"
        );

        ImGui::Separator();

        ImGui::Combo("PGA Gain", &tmp_measure_captures.pga_gain_combo, "x1\0""x5\0");

        ImGui::Separator();

        ImGui::InputText(
            "Number of Settling Time Cycles",
            tmp_measure_captures.settling_time_cycles_num.data(),
            tmp_measure_captures.settling_time_cycles_num.size(),
            input_flags
        );
        ImGui::Combo(
            "Settling Time Cycles Multiplier",
            &tmp_measure_captures.settling_time_cycles_multiplier_combo,
            "x1\0""x2\0""x4\0"
        );

        ImGui::Separator();

        ImGui::Combo("System Clock Source", &tmp_measure_captures.sysclk_src_combo, "Internal\0""External\0");
        ImGui::InputText(
            "System Clock Frequency",
            tmp_measure_captures.sysclk_freq.data(),
            tmp_measure_captures.sysclk_freq.size(),
            ImGuiInputTextFlags_ReadOnly
        );

        ImGui::Separator(); 

        static bool configured = false;
        if(ImGui::Button("Configure")) {
            std::thread(
                configure_cb,
                measure_captures.load()->config.to_raw_array(),
                std::ref(esp32_ad5933),
                std::ref(configured)
            ).detach();
        }

        if(configured == true) {
            ImGui::SameLine(); if(ImGui::Button("Freq Sweep End")) {
                std::thread(
                    freq_sweep_end_cb,
                    std::ref(esp32_ad5933),
                    std::ref(configured)
                ).detach();
            } 

            static std::atomic<float> calibrating_progress_bar_fraction { 0.0f };
            if(calibrating == false && sweeping == false) {
                if(ImGui::Button("Calibrate")) {
                    calibrating = true;
                    calibrated = false;
                    calibrating_progress_bar_fraction.store(0.0f);
                    std::thread(calibrate_cb,
                        measure_captures.load()->config.get_num_of_inc(),
                        measure_captures.load()->calibration_impedance,
                        std::ref(calibrating),
                        std::ref(calibrated),
                        std::ref(calibrating_progress_bar_fraction),
                        std::ref(esp32_ad5933),
                        std::ref(another_calibration_data),
                        std::ref(raw_calibration_data)
                    ).detach();
                }
            } else if(sweeping == false) {
                Spinner::Spinner("Calibrating", 10.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                ImGui::ProgressBar(calibrating_progress_bar_fraction.load());
            }

            static std::atomic<float> sweeping_progress_bar_fraction { 0.0f };
            if(calibrated == true) {
                ImGui::SameLine(); if(ImGui::Button("View Calibration Data")) {
                    plot_calibration_window_enable = true;
                }
                static bool periodically_sweeping = false;
                ImGui::Checkbox("Periodic Sweep", &periodically_sweeping);
                if(sweeping == false) {
                    if(ImGui::Button("Start Sweep")) {
                        sweeping = true;
                        sweeped = false;
                        sweeping_progress_bar_fraction.store(0.0f);
                        std::thread(start_sweep_cb,
                            measure_captures.load()->config.get_num_of_inc(),
                            std::ref(sweeping),
                            std::ref(sweeped),
                            std::ref(sweeping_progress_bar_fraction),
                            std::ref(esp32_ad5933),
                            std::ref(periodically_sweeping),
                            std::ref(another_calibration_data),
                            std::ref(raw_measurement_data),
                            std::ref(another_measurement_data)
                        ).detach();
                    }
                } else if(calibrating == false) {
                    Spinner::Spinner("Measuring", 10.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    ImGui::ProgressBar(sweeping_progress_bar_fraction.load());
                }
            }

            if(sweeped == true) {
                ImGui::SameLine(); if(ImGui::Button("View Frequency Sweep Data")) {
                    plot_freq_sweep_window_enable = true;
                }
            }

            ImGui::Separator(); 
        }

        ImGui::End();
        tmp_measure_captures.update_config();
        measure_captures.store(std::make_shared<Windows::Captures::Measurement>(tmp_measure_captures));
    }
}

namespace GUI {
    void calibration_raw_data_plots(
        std::vector<float> &frequency_vector,
        std::vector<AD5933::Data> &raw_calibration_data
    ) {
        if(ImPlot::BeginPlot("Calibration Raw Real Data")) {
            ImPlot::SetupAxes("f [Hz]", "REAL_DATA");
            std::vector<float> real_data_vector(raw_calibration_data.size());
            std::transform(raw_calibration_data.begin(), raw_calibration_data.end(), real_data_vector.begin(), [](AD5933::Data &e) { return static_cast<float>(e.get_real_data()); });
            ImPlot::PlotLine("REAL_DATA [1/Ohm]", frequency_vector.data(), real_data_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
         if(ImPlot::BeginPlot("Calibration Raw Imag Data")) {
            ImPlot::SetupAxes("f [Hz]", "IMAG_DATA");
            std::vector<float> imag_data_vector(raw_calibration_data.size());
            std::transform(raw_calibration_data.begin(), raw_calibration_data.end(), imag_data_vector.begin(), [](AD5933::Data &e) { return static_cast<float>(e.get_imag_data()); });
            ImPlot::PlotLine("IMAG_DATA [1/Ohm]", frequency_vector.data(), imag_data_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
    }

    void calibration_calculated_data_plots(
        std::vector<float> &frequency_vector,
        std::vector<AD5933::Data> &raw_calibration_data
    ) {
        if(ImPlot::BeginPlot("Calibration Calculated Magnitude")) {
            ImPlot::SetupAxes("f [Hz]", "RAW_MAGNITUDE");
            std::vector<float> raw_magnitude_vector(raw_calibration_data.size());
            std::transform(raw_calibration_data.begin(), raw_calibration_data.end(), raw_magnitude_vector.begin(), [](AD5933::Data &e) { return e.get_raw_magnitude<float>(); });
            ImPlot::PlotLine("RAW_MAGNITUDE [1/Ohm]", frequency_vector.data(), raw_magnitude_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
         if(ImPlot::BeginPlot("Calibration Calculated Phase")) {
            ImPlot::SetupAxes("f [Hz]", "RAW_PHASE");
            std::vector<float> raw_phase_vector(raw_calibration_data.size());
            std::transform(raw_calibration_data.begin(), raw_calibration_data.end(), raw_phase_vector.begin(), [](AD5933::Data &e) { return e.get_raw_phase<float>(); });
            ImPlot::PlotLine("RAW_PHASE [rad]", frequency_vector.data(), raw_phase_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
    }

    void calibration_data_plots(
        std::vector<float> &frequency_vector,
        std::vector<AD5933::Calibration<float>> &calibration_data
    ) {
        if(ImPlot::BeginPlot("Calibration Gain Factor")) {
            ImPlot::SetupAxes("f [Hz]", "GAIN_FACTOR");
            std::vector<float> gain_factor_vector(calibration_data.size());
            std::transform(
                calibration_data.begin(),
                calibration_data.end(),
                gain_factor_vector.begin(),
                [](AD5933::Calibration<float> &e) {
                    return static_cast<float>(e.get_gain_factor());
                }
            );
            ImPlot::PlotLine("GAIN_FACTOR", frequency_vector.data(), gain_factor_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
        if(ImPlot::BeginPlot("Calibration System Phase")) {
            ImPlot::SetupAxes("f [Hz]", "SYSTEM_PHASE");
            std::vector<float> raw_phase_vector(calibration_data.size());
            std::transform(
                calibration_data.begin(),
                calibration_data.end(),
                raw_phase_vector.begin(),
                [](AD5933::Calibration<float> &e) {
                    return static_cast<float>(e.get_system_phase()); 
                }
            );
            ImPlot::PlotLine("SYSTEM_PHASE [rad]", frequency_vector.data(), raw_phase_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
    }

    void create_plot_calibration_window(
        std::atomic<std::shared_ptr<Windows::Captures::Measurement>> &measure_captures,
        bool &running,
        std::vector<AD5933::Calibration<float>> &another_calibration_data,
        std::vector<AD5933::Data> &raw_calibration_data
    ) {
        ImPlot::CreateContext();
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(600, 750), ImGuiCond_FirstUseEver);
        ImGui::Begin("Calibration Plots", &running);

        const auto start_freq = measure_captures.load()->config.get_start_freq();
        const auto inc_freq = measure_captures.load()->config.get_inc_freq();
        std::vector<float> frequency_vector(another_calibration_data.size());
        std::generate(
            frequency_vector.begin(),
            frequency_vector.end(),
            [start_freq, inc_freq, n = 0.0] () mutable {
                return static_cast<float>(start_freq.unwrap() + ((n++) * inc_freq.unwrap()));
            }
        );

        if(ImGui::BeginTabBar("Calibration_PlotsBar")) {
            if (ImGui::BeginTabItem("RAW_DATA")) {
                calibration_raw_data_plots(frequency_vector, raw_calibration_data);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("CALCULATED_DATA")) {
                calibration_calculated_data_plots(frequency_vector, raw_calibration_data);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("CALIBRATION_DATA")) {
                calibration_data_plots(frequency_vector, another_calibration_data);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();

        ImGui::End();
    }
}

namespace GUI {
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

    void create_freq_sweep_window_enable(
        std::atomic<std::shared_ptr<Windows::Captures::Measurement>> &measure_captures,
        bool &running,
        std::vector<AD5933::Data> &raw_measurement_data,
        std::vector<AD5933::Measurement<float>> &measurement_data
    ) {
        ImPlot::CreateContext();
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(600, 750), ImGuiCond_FirstUseEver);
        ImGui::Begin("Frequency Sweep Plots", &running);

        const auto start_freq = measure_captures.load()->config.get_start_freq();
        const auto inc_freq = measure_captures.load()->config.get_inc_freq();
        std::vector<float> frequency_vector(measurement_data.size());
        std::generate(
            frequency_vector.begin(),
            frequency_vector.end(),
            [start_freq, inc_freq, n = 0.0] () mutable {
                return static_cast<float>(start_freq.unwrap() + ((n++) * inc_freq.unwrap()));
            }
        );

        if(ImGui::BeginTabBar("FREQ_SWEEP_BAR")) {
            if (ImGui::BeginTabItem("RAW_DATA")) {
                freq_sweep_raw_data_plots(frequency_vector, raw_measurement_data);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("CALCULATED_DATA")) {
                freq_sweep_calculated_data_plots(frequency_vector, raw_measurement_data);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("CORRECTED_DATA")) {
                freq_sweep_corrected_data_plots(frequency_vector, measurement_data);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("IMPEDANCE_DATA")) {
                freq_sweep_impedance_data_plots(frequency_vector, measurement_data);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();

        ImGui::End();
    }
}

namespace GUI {
    void create_main(
        std::optional<ESP32_AD5933> &esp32_ad5933,
        std::atomic<std::shared_ptr<Windows::Captures::Measurement>> &measure_captures
    ) {
        static ImVec2 measurement_window_size = ImGui::GetIO().DisplaySize;
        //ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        //ImGui::SetNextWindowSize(measurement_window_size);
        static bool plot_calibration_window_enable = false;
        static bool plot_freq_sweep_window_enable = false;
        static std::vector<AD5933::Calibration<float>> another_calibration_data;
        static std::vector<AD5933::Data> raw_calibration_data;
        static std::atomic<int> debug_window_enable { 0 };
        static std::vector<AD5933::Data> raw_measurement_data;
        static std::vector<AD5933::Measurement<float>> another_measurement_data;
        measurement_window(
            measure_captures,
            measurement_window_size,
            esp32_ad5933,
            plot_calibration_window_enable,
            plot_freq_sweep_window_enable,
            another_calibration_data,
            raw_calibration_data,
            debug_window_enable,
            raw_measurement_data,
            another_measurement_data
        );
        if(debug_window_enable.load()) {
            //ImGui::SetNextWindowPos(ImVec2(measurement_window_size.x, 0.0f));
            //ImGui::SetNextWindowSize(measurement_window_size);
            static std::atomic<std::shared_ptr<Windows::Captures::HexDebugReadWriteRegisters>> hex_dbg_rw_captures { std::make_shared<Windows::Captures::HexDebugReadWriteRegisters>() };
            debug_registers_window(esp32_ad5933, hex_dbg_rw_captures);
        }
        if(plot_calibration_window_enable) {
            create_plot_calibration_window(measure_captures, plot_calibration_window_enable, another_calibration_data, raw_calibration_data);
        }
        if(plot_freq_sweep_window_enable) {
            create_freq_sweep_window_enable(measure_captures, plot_freq_sweep_window_enable, raw_measurement_data, another_measurement_data);
        }
    }
}

namespace GUI {
    void run(bool &done, boost::process::child& ble_client, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
        SDL_Window* window;
        SDL_Renderer* renderer;
        {
            const auto ret { Boilerplate::init() };
            window = std::get<0>(ret);
            renderer = std::get<1>(ret);
        }

        const ImVec4 clear_color { 0.45f, 0.55f, 0.60f, 1.00f };

        Boilerplate::process_events(window, done);
        Boilerplate::start_new_frame();
        Windows::MenuBarEnables menu_bar_enables;
        ImGuiID top_id = Windows::top_with_dock_space(menu_bar_enables);
        Windows::DockspaceIDs top_ids { Windows::split_left_center(top_id) };

        int selected = -1;
        int client_index = -1;
        Windows::ble_client(menu_bar_enables.ble_client, top_ids.left, selected, shm, client_index);
        Boilerplate::render(renderer, clear_color);
        std::unique_ptr<Windows::Client> empty_client = std::make_unique<Windows::Client>();

        while(done == false) {
            Boilerplate::process_events(window, done);
            Boilerplate::start_new_frame();
            top_id = Windows::top_with_dock_space(menu_bar_enables);

            Windows::ble_client(menu_bar_enables.ble_client, top_ids.left, selected, shm, client_index);

            std::visit([&](auto&& active_state) {
                using T_Decay = std::decay_t<decltype(active_state)>;
                if constexpr (std::is_same_v<T_Decay, BLE_Client::Discovery::States::using_esp32_ad5933>) {
                    if(client_index > -1) {
                        if(empty_client == nullptr) {
                            empty_client = std::make_unique<Windows::Client>();
                        } else {
                            Windows::client1(0, top_ids.center, *empty_client, menu_bar_enables, shm, client_index);
                        }
                        if(empty_client->enable == false) {
                            shm->cmd_deque->push_back(BLE_Client::Discovery::Events::stop_using_esp32_ad5933{});
                            shm->cmd_deque->push_back(BLE_Client::Discovery::Events::disconnect{});
                            client_index = -1;
                            empty_client.~unique_ptr();
                        }
                    }
                }
            }, *shm->active_state);

            Boilerplate::render(renderer, clear_color);
        }

        Boilerplate::shutdown(renderer, window);
    }
}
