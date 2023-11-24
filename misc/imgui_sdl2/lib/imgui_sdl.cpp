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

#include "trielo.hpp"
#include "fmt/core.h"
#include "fmt/color.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "implot.h"
#include "magic_packets.hpp"
#include "types.hpp"

#include "dark_mode_switcher.hpp"
#include "imgui_sdl.hpp"
#include "ble.hpp"
#include "ad5933_config.hpp"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

namespace ImGuiSDL {
    // ImGuiSDL utility boilerplate functions
    static const auto sdl_error_lambda = []() { std::cout << SDL_GetError() << std::endl; };

    bool check_version() {
        return IMGUI_CHECKVERSION();
    }

    std::tuple<SDL_Window*, SDL_Renderer*> init() {
        Trielo::trieloxit_lambda<SDL_Init>(Trielo::OkErrCode(0), sdl_error_lambda, SDL_INIT_VIDEO | SDL_INIT_TIMER);
        Trielo::trielo_lambda<SDL_SetHint>(Trielo::OkErrCode(SDL_TRUE), sdl_error_lambda, SDL_HINT_IME_SHOW_UI, "1");

        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN);
        SDL_Window* window = Trielo::trieloxit_lambda<SDL_CreateWindow>(
            Trielo::FailErrCode(static_cast<SDL_Window*>(nullptr)),
            sdl_error_lambda,
            "AD5933",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            1280,
            720,
            window_flags
        );

        SDL_Renderer* renderer = Trielo::trieloxit_lambda<SDL_CreateRenderer>(
            Trielo::FailErrCode(static_cast<SDL_Renderer*>(nullptr)),
            sdl_error_lambda,
            window,
            -1,
            SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED
        );
        {
            SDL_RendererInfo info;
            Trielo::trielo_lambda<SDL_GetRendererInfo>(
                Trielo::OkErrCode(0),
                sdl_error_lambda,
                renderer,
                &info
            );
            SDL_Log("Current SDL_Renderer: %s", info.name);
        }

        Trielo::trieloxit<check_version>(Trielo::OkErrCode(true));
        if(ImGui::CreateContext() == nullptr) {
            std::cout << "ImGui failed to create context\n";
        }

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);

        /*
        DarkModeSwitcher dark_mode_switcher { window };
        if(DarkModeSwitcher::isGlobalDarkModeActive()) {
            ImGui::StyleColorsDark();
        } else {
            ImGui::StyleColorsLight();
        }
        */
        //Commented out because of a bug in a lambda capture of this and segfault on when darkModeRevoker is fired
        //dark_mode_switcher.enable_dynamic_switching();

        ImGui::StyleColorsDark();
        SDL_MaximizeWindow(window);
        return std::tuple { window, renderer };
    } 

    void shutdown(SDL_Renderer* renderer, SDL_Window* window) {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    inline void render(SDL_Renderer* renderer, ImVec4& clear_color) {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Render();
        if(SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y) != 0) {
            fmt::print(fmt::fg(fmt::color::yellow), "WARNING: SDL_RenderSetScale failed");
            sdl_error_lambda();
        }
        if(SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255)) != 0) {
            fmt::print(fmt::fg(fmt::color::yellow), "WARNING: SDL_SetRenderDrawColor failed");
            sdl_error_lambda();
        }
        if(SDL_RenderClear(renderer) != 0) {
            fmt::print(fmt::fg(fmt::color::yellow), "WARNING: SDL_RenderClear failed\n");
            sdl_error_lambda();
        }
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }

    inline void process_events(SDL_Window* window, bool *done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if(event.type == SDL_QUIT) {
                *done = true;
            }
            if(event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(window)) {
                *done = true;
            }
        }
    }

    inline void start_new_frame() {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    inline void connecting_window() {
        static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
        ImGui::Begin("ESP32_AD5933", NULL, flags);                          // Create a window called "Hello, world!" and append into it.
        static uint8_t dot_count = 0;
        switch(dot_count) {
            case 0:
                ImGui::Text("Connecting.");
                break;
            case 1:
                ImGui::Text("Connecting..");
                break;
            case 2:
                ImGui::Text("Connecting...");
                break;
            default:
                ImGui::Text("Connecting");
                break;
        }
        dot_count++;
        if(dot_count > 2) {
            dot_count = 0;
        }
        ImGui::End();
    }

    inline void imgui_create_connecting_window() {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        connecting_window();
    }
}

namespace ImGuiSDL {
    // Demos - unused for now
    inline void periodic_temperature_measurement_demo_window() {
        static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
        ImGui::Begin("Periodic Temperature Measurement Demo", NULL, flags);                          // Create a window called "Hello, world!" and append into it.
        static bool toggle_capture;
        bool toggle_capture_prev = toggle_capture;
        ImGui::Checkbox("Toggle Temperature Measurement: ", &toggle_capture);
        if(esp32_ad5933.has_value() && (toggle_capture != toggle_capture_prev)) {
            if(toggle_capture == true) {
                esp32_ad5933.value().subscribe_to_body_composition_measurement_indicate();
            } else {
                esp32_ad5933.value().unsubscribe_from_body_composistion_measurement();
            }
        }
        ImGui::SameLine();
        if(esp32_ad5933.has_value()) {
            ImGui::Text(esp32_ad5933.value().temp_payload.value_or("0xFFFF'FFFF").c_str());
        }
        ImGui::End();
    }
}

namespace ImGuiSDL {
    // Related to dbg_registers_window   
    template<size_t N>
    class DebugReadWriteRegistersCaptures {
    public:
        std::array<char, N> ctrl_HB;
        std::array<char, N> ctrl_LB;
        std::array<char, N> freq_start_HB;
        std::array<char, N> freq_start_MB;
        std::array<char, N> freq_start_LB;
        std::array<char, N> freq_inc_HB;
        std::array<char, N> freq_inc_MB;
        std::array<char, N> freq_inc_LB;
        std::array<char, N> num_of_inc_HB;
        std::array<char, N> num_of_inc_LB;
        std::array<char, N> num_of_settling_time_cycles_HB;
        std::array<char, N> num_of_settling_time_cycles_LB;
        std::array<char, N> status_capture;
        std::array<char, N> temp_data_HB;
        std::array<char, N> temp_data_LB;
        std::array<char, N> real_data_HB;
        std::array<char, N> real_data_LB;
        std::array<char, N> imag_data_HB;
        std::array<char, N> imag_data_LB;
        std::array<std::array<char, N>*, 19> all {
            &ctrl_HB,
            &ctrl_LB,
            &freq_start_HB,
            &freq_start_MB,
            &freq_start_LB,
            &freq_inc_HB,
            &freq_inc_MB,
            &freq_inc_LB,
            &num_of_inc_HB,
            &num_of_inc_LB,
            &num_of_settling_time_cycles_HB,
            &num_of_settling_time_cycles_LB,
            &status_capture,
            &temp_data_HB,
            &temp_data_LB,
            &real_data_HB,
            &real_data_LB,
            &imag_data_HB,
            &imag_data_LB,
        };
        AD5933_Config config;
        AD5933_Data data;

        DebugReadWriteRegistersCaptures() = default;

        void update_config() {
            auto stoul_lambda = [](const std::array<char, 5>& hex_array) -> std::optional<uint8_t> {
                try {
                    // Extract the numeric value from the string
                    uint8_t result = static_cast<uint8_t>(std::stoul(std::string { hex_array.begin(), hex_array.end() }, nullptr, 16)); // Convert hexadecimal string to unsigned long
                    return result;
                } catch (...) {
                    return std::nullopt; // Return empty optional on conversion failure
                }
            };
            const std::array<uint8_t, 12> tmp_raw_array {
                stoul_lambda(ctrl_HB).value_or(0xFF),
                stoul_lambda(ctrl_LB).value_or(0xFF),
                stoul_lambda(freq_start_HB).value_or(0xFF),
                stoul_lambda(freq_start_MB).value_or(0xFF),
                stoul_lambda(freq_start_LB).value_or(0xFF),
                stoul_lambda(freq_inc_HB).value_or(0xFF),
                stoul_lambda(freq_inc_MB).value_or(0xFF),
                stoul_lambda(freq_inc_LB).value_or(0xFF),
                stoul_lambda(num_of_inc_HB).value_or(0xFF),
                stoul_lambda(num_of_inc_LB).value_or(0xFF),
                stoul_lambda(num_of_settling_time_cycles_HB).value_or(0xFF),
                stoul_lambda(num_of_settling_time_cycles_LB).value_or(0xFF),
            };
            config = AD5933_Config { tmp_raw_array };
        }
    };

    class HexDebugReadWriteRegistersCaptures : public DebugReadWriteRegistersCaptures<5> {

    };

    class BinDebugReadWriteRegistersCaptures : public DebugReadWriteRegistersCaptures<12> {

    };

    std::atomic<std::shared_ptr<HexDebugReadWriteRegistersCaptures >> hex_dbg_rw_captures { std::make_shared<HexDebugReadWriteRegistersCaptures >() };
    std::atomic<int> debug_window_enable { 0 };

    void update_hex_dbg_rw_captures(const std::string &data) {
        auto tmp_hex_dbg_rw_captures = *(hex_dbg_rw_captures.load());

        std::array<std::string, 19> oss_array {};
        for(size_t i = 0; i < data.size(); i++) {
            std::ostringstream temp_oss;
            temp_oss << "0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(data[i]));
            oss_array[i] = temp_oss.str();
        }

        size_t i = 0;
        for(const auto &oss_content: oss_array) {
            std::array<char, 5> temp_array{}; // Initialize an array of chars with size 5
            std::copy(oss_content.begin(), oss_content.begin() + std::min(oss_content.size(), temp_array.size()), temp_array.begin());
            *(tmp_hex_dbg_rw_captures.all[i]) = temp_array;
            i++;
        }

        std::array<uint8_t, 12> config_data_array;
        std::transform(
            data.begin(),
            data.begin() + config_data_array.size(),
            config_data_array.begin(), 
            [](char c) { 
                return static_cast<uint8_t>(c);
            }
        );
        tmp_hex_dbg_rw_captures.config = AD5933_Config { config_data_array };

        std::array<uint8_t, 7> data_data_array;
        std::transform(
            data.begin() + config_data_array.size(),
            data.end(),
            data_data_array.begin(), 
            [](char c) { 
                return static_cast<uint8_t>(c);
            }
        );
        tmp_hex_dbg_rw_captures.data = AD5933_Data { data_data_array };
        hex_dbg_rw_captures.store(std::make_shared<HexDebugReadWriteRegistersCaptures>(tmp_hex_dbg_rw_captures));
    }

    void send_debug_start_req_thread_cb() {
        esp32_ad5933.value().send(std::string(MagicPackets::Debug::Command::start.begin(), MagicPackets::Debug::Command::start.end()));
    }

    void send_debug_end_req_thread_cb() {
        esp32_ad5933.value().send(std::string(MagicPackets::Debug::Command::end.begin(), MagicPackets::Debug::Command::end.end()));
    }
   
    void send_dump_all_registers_req_thread_cb() {
        esp32_ad5933.value().send(std::string(MagicPackets::Debug::Command::dump_all_registers.begin(), MagicPackets::Debug::Command::dump_all_registers.end()));
        const auto rx_register_info = esp32_ad5933.value().rx_payload.read();
        esp32_ad5933.value().rx_payload.clean();
    
        esp32_ad5933.value().print_mtu();
        for(size_t i = 0; i < rx_register_info.size(); i++) {
            std::printf("Register[%zu]: 0x%02X\n", i, static_cast<uint8_t>(rx_register_info[i]));
        }
        update_hex_dbg_rw_captures(std::string(rx_register_info.begin(), rx_register_info.begin() + 19));
    }

    void dbg_registers_thread_cb() {
        while(debug_window_enable.load() > 0) {
            debug_window_enable.wait(debug_window_enable);
            if(debug_window_enable.load() == 2) {
                std::cout << "ImGuiSDL: Loading rx_payload content\n";
                debug_window_enable.store(1);
            }
        }

        debug_window_enable.store(0);
        std::cout << "ImGuiSDL: ending dbg_registers_thread_cb\n";
    }

    void program_all_registers_thread_cb() {
        const auto tmp_hex_dbg_rw_captures = *(hex_dbg_rw_captures.load());
        auto buf_to_send = MagicPackets::Debug::Command::program_all_registers;
        const auto raw_message = tmp_hex_dbg_rw_captures.config.get_ad5933_config_message_raw();
        std::copy(raw_message.begin(), raw_message.end(), buf_to_send.begin());

        esp32_ad5933.value().send(std::string(buf_to_send.begin(), buf_to_send.end()));

        const auto rx_message = esp32_ad5933.value().rx_payload.read();
        esp32_ad5933.value().rx_payload.clean();

        std::array<uint8_t, 20> rx_message_array;
        std::copy(rx_message.begin(), rx_message.end(), rx_message_array.begin());
        if(rx_message_array == MagicPackets::Debug::Command::program_all_registers) {
            std::cout << "Success: program_all_registers_thread_cb\n";
        } else {
            std::cout << "ERROR: program_all_registers_thread_cb\n";
        }
        std::thread send_dump_all_registers_thread(send_dump_all_registers_req_thread_cb);
        send_dump_all_registers_thread.join();
    }

    void control_command_thread_cb(const ControlHB::CommandOrMask command) {
        const auto tmp_hex_dbg_rw_captures = *(hex_dbg_rw_captures.load());
        auto buf_to_send = MagicPackets::Debug::Command::control_HB_command;
        buf_to_send[0] = static_cast<uint8_t>(command);

        esp32_ad5933.value().send(std::string(buf_to_send.begin(), buf_to_send.end()));

        const auto rx_message = esp32_ad5933.value().rx_payload.read();
        esp32_ad5933.value().rx_payload.clean();

        std::array<uint8_t, 20> rx_message_array;
        std::copy(rx_message.begin(), rx_message.end(), rx_message_array.begin());
        if(rx_message_array == MagicPackets::Debug::Command::control_HB_command) {
            std::cout << "Success: control_command_thread_cb\n";
        } else {
            std::cout << "ERROR: control_command_thread_cb\n";
        }
        std::thread send_dump_all_registers_thread(send_dump_all_registers_req_thread_cb);
        send_dump_all_registers_thread.join();
    }

    static const auto default_config = AD5933_Config::get_default();
    void debug_registers_window() {
        static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
        ImGui::Begin("Register Debug", NULL, flags);                          // Create a window called "Hello, world!" and append into it.
        static bool hex_toggle_capture = true;

        auto tmp_hex_dbg_rw_captures = *(hex_dbg_rw_captures.load());

        if(ImGui::Button("HEX/BIN Toggle")) {
            hex_toggle_capture = !hex_toggle_capture;
        }; ImGui::SameLine();

        static bool debug_started = false;
        if(ImGui::Button("Debug Start") && debug_started == false) {
            debug_started = true;
            std::thread debug_start_thread(send_debug_start_req_thread_cb);
            debug_start_thread.detach();
        }

        if(debug_started == true) {
            ImGui::SameLine();
            if(ImGui::Button("Dump All Registers")) {
                std::thread dump_all_registers_thread (send_dump_all_registers_req_thread_cb);
                dump_all_registers_thread.detach();
            }; ImGui::SameLine();

            if(ImGui::Button("Program Read/Write Registers")) {
                std::thread program_all_registers_thread(program_all_registers_thread_cb);
                program_all_registers_thread.detach();
            }; ImGui::SameLine();

            if(ImGui::Button("Debug End")) {
                std::thread debug_end_thread(send_debug_end_req_thread_cb);
                debug_end_thread.detach();
                debug_started = false;
            }
        }

        ImGui::Separator();

        ImGui::Text("Decoded Register Values");
        ImGui::Separator();

        ImGui::Text("ControlHB:");
        ImGui::Text("\tControlHB Command: %s", ControlHB_CommandOrMaskStringMap.at(tmp_hex_dbg_rw_captures.config.get_command()));
        ImGui::Text("\tExcitation Output Voltage Range: %s", ControlHB_OutputVoltageRangeOrMaskStringMap.at(tmp_hex_dbg_rw_captures.config.get_voltage_range()));
        ImGui::Text("\tPGA Gain: %s", ControlHB_PGA_GainOrMaskStringMap.at(tmp_hex_dbg_rw_captures.config.get_pga_gain()));
        ImGui::Separator();

        ImGui::Text("ControlLB:");
        ImGui::Text("\tReset: %s", tmp_hex_dbg_rw_captures.config.get_reset() == true ? "true" : "false");
        ImGui::Text("\tSystem Clock Source: %s", ControlLB_SYSCLK_SRC_OrMaskStringMap.at(tmp_hex_dbg_rw_captures.config.get_sysclk_src())); 
        ImGui::Separator();

        ImGui::Text("Start Frequency: %f", tmp_hex_dbg_rw_captures.config.get_start_freq());
        ImGui::Text("Frequency Increment: %f", tmp_hex_dbg_rw_captures.config.get_inc_freq());
        ImGui::Text("Number of Increments: %u", tmp_hex_dbg_rw_captures.config.get_num_of_inc().value);
        ImGui::Text("Number of Settling Time Cycles: %u", tmp_hex_dbg_rw_captures.config.get_settling_time_cycles_number().value);
        ImGui::Text("Settling Time Cycles Multiplier: %s", SettlingTimeCyclesMultiplierOrMaskStringMap.at(tmp_hex_dbg_rw_captures.config.get_settling_time_cycles_multiplier()));
        ImGui::Separator();

        ImGui::Text("Status: %s", STATUS_OrMaskStringMap.at(tmp_hex_dbg_rw_captures.data.get_status()));
        ImGui::Separator();

        ImGui::Text("Temperature Data: %f", tmp_hex_dbg_rw_captures.data.get_temperature());
        ImGui::Text("Real Data: %s", tmp_hex_dbg_rw_captures.data.get_real_data().c_str());
        ImGui::Text("Imaginary Data: %s", tmp_hex_dbg_rw_captures.data.get_imag_data().c_str());
        ImGui::Separator();

        ImGui::Text("Read/Write Registers");

        if(hex_toggle_capture) {
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
            ImGui::InputText("NUM_OF_SETTLING_TIME_CYCLES_HB (0x8A)", tmp_hex_dbg_rw_captures.num_of_settling_time_cycles_HB.data(), tmp_hex_dbg_rw_captures.num_of_settling_time_cycles_HB.size());
            ImGui::InputText("NUM_OF_SETTLING_TIME_CYCLES_LB (0x8B)", tmp_hex_dbg_rw_captures.num_of_settling_time_cycles_LB.data(), tmp_hex_dbg_rw_captures.num_of_settling_time_cycles_LB.size());

            ImGui::Separator();

            {
            ImGui::Text("Send Control Register Command Controls");
            if(ImGui::Button("Power-down mode")) {
                std::thread control_command_thread(control_command_thread_cb, ControlHB::CommandOrMask::POWER_DOWN_MODE);
            } ImGui::SameLine();
                if(ImGui::Button("Standby mode")) {
                    std::thread control_command_thread(control_command_thread_cb, ControlHB::CommandOrMask::STANDBY_MODE);
                } ImGui::SameLine();
                if(ImGui::Button("No operation (NOP_0)")) {
                    std::thread control_command_thread(control_command_thread_cb, ControlHB::CommandOrMask::NOP_0);
                }

            if(ImGui::Button("Measure temperature")) {
                std::thread control_command_thread(control_command_thread_cb, ControlHB::CommandOrMask::MEASURE_TEMPERATURE);
            }

            if(ImGui::Button("Initialize with start frequency")) {
                std::thread control_command_thread(control_command_thread_cb, ControlHB::CommandOrMask::INITIALIZE_WITH_START_FREQUENCY);
            } ImGui::SameLine();
                if(ImGui::Button("Start frequency sweep")) {
                    std::thread control_command_thread(control_command_thread_cb, ControlHB::CommandOrMask::START_FREQUENCY_SWEEP);
                } ImGui::SameLine();
                if(ImGui::Button("Increment frequency")) {
                    std::thread control_command_thread(control_command_thread_cb, ControlHB::CommandOrMask::INCREMENT_FREQUENCY);
                } ImGui::SameLine();
                if(ImGui::Button("Repeat frequency")) {
                    std::thread control_command_thread(control_command_thread_cb, ControlHB::CommandOrMask::REPEAT_FREQUENCY);
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
        } else {
            ImGui::Text("UNIMPLEMENTED");
        }

        tmp_hex_dbg_rw_captures.update_config();
        hex_dbg_rw_captures.store(std::make_shared<HexDebugReadWriteRegistersCaptures>(tmp_hex_dbg_rw_captures));
        ImGui::End();
    }
}

namespace ImGuiSDL {
    // Related to measurement_window  
    class MeasurementWindowCaptures {
    public:
        AD5933_Config config;

        std::array<char, 7> freq_start { 0x00 };
        std::array<char, 7> freq_inc { 0x00 };
        std::array<char, 4> num_of_inc { 0x00 };
        std::array<char, 4> settling_time_cycles_num { 0x00 };
        int settling_time_cycles_multiplier_combo = 0;
        int voltage_range_combo = 0;
        int pga_gain_combo = 0;
        int sysclk_src_combo = 0;

        std::array<char, 9> sysclk_freq { 0x00 };

        MeasurementWindowCaptures() :
            config { AD5933_Config() }
        {} 

        void update_config() {
            auto stoul_lambda = [](const std::string &num_str) -> std::optional<unsigned long> {
                try {
                    size_t pos;
                    if(num_str.empty()) {
                        return std::nullopt;
                    }
                    unsigned long number = std::stoul(num_str, &pos);

                    if(pos < std::strlen(num_str.c_str())) {
                        std::cout << "Conversion failed. Not the entire string was converted." << std::endl;
                        return std::nullopt; // Return empty optional if conversion is incomplete
                    } else {
                        return number;
                    }
                } catch (const std::out_of_range& e) {
                    std::cout << "Exception caught: " << e.what() << std::endl;
                    // Handle the out_of_range exception here
                } catch (const std::invalid_argument& e) {
                    std::cout << "Invalid argument exception caught: " << e.what() << std::endl;
                    // Handle the invalid_argument exception here
                }
                    return std::nullopt; // Return empty optional in case of exceptions
            };
            config = AD5933_Config {
                config.get_command(),
                std::next(ControlHB_OutputVoltageRangeOrMaskStringMap.begin(), settling_time_cycles_multiplier_combo)->first,
                std::next(ControlHB_PGA_GainOrMaskStringMap.begin(), pga_gain_combo)->first,
                std::next(ControlLB_SYSCLK_SRC_OrMaskStringMap.begin(), sysclk_src_combo)->first,
                stoul_lambda(std::string(freq_start.begin(), freq_start.end())).value_or(0xFF'FF'FF),
                stoul_lambda(std::string(freq_inc.begin(), freq_inc.end())).value_or(0xFF'FF'FF),
                uint9_t { static_cast<uint16_t>(stoul_lambda(std::string(num_of_inc.begin(), num_of_inc.end())).value_or(0b1'1111'1111)) },
                uint9_t { static_cast<uint16_t>(stoul_lambda(std::string(settling_time_cycles_num.begin(), settling_time_cycles_num.end())).value_or(0b1'1111'1111)) },
                std::next(SettlingTimeCyclesMultiplierOrMaskStringMap.begin(), settling_time_cycles_multiplier_combo)->first
            };
        }
    };

    std::atomic<std::shared_ptr<MeasurementWindowCaptures>> measure_captures { std::make_shared<MeasurementWindowCaptures>() };
    inline void measurement_window(ImVec2 &measurement_window_size) {
        static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
        ImGui::Begin("Measurement", NULL, flags);                          // Create a window called "Hello, world!" and append into it.
        if(ImGui::Button("Toggle Register Debug") == true) {
            if(debug_window_enable.load() == false) {
                measurement_window_size.x /= 2;
                debug_window_enable.store(1);
                std::thread dbg_registers_thread(dbg_registers_thread_cb);
                dbg_registers_thread.detach();
            } else {
                measurement_window_size = ImGui::GetIO().DisplaySize;
                debug_window_enable.store(0);
            }
            debug_window_enable.notify_all();
        }

        auto tmp_measure_captures = *(measure_captures.load());

        ImGui::Separator(); 
        ImGui::Text("Sweep Parameters");
        ImGui::Separator();
        ImGui::InputText("Start Frequency", tmp_measure_captures.freq_start.data(), tmp_measure_captures.freq_start.size());
        ImGui::InputText("Increment Frequency", tmp_measure_captures.freq_inc.data(), tmp_measure_captures.freq_inc.size());
        ImGui::InputText("Number of Increments", tmp_measure_captures.num_of_inc.data(), tmp_measure_captures.num_of_inc.size());
        ImGui::InputText("Number of Settling Time Cycles", tmp_measure_captures.settling_time_cycles_num.data(), tmp_measure_captures.settling_time_cycles_num.size());

        ImGui::Combo("Settling Time Cycles Multiplier", &tmp_measure_captures.settling_time_cycles_multiplier_combo, "x1\0x2\0x4");

        ImGui::Combo("Output Excitation Voltage Range", &tmp_measure_captures.voltage_range_combo, "2 Vppk""\0""1 Vppk""\0""400 mVppk""\0""200 mVppk");
        ImGui::Combo("PGA Gain", &tmp_measure_captures.pga_gain_combo, "x1\0x5");
        ImGui::Combo("System Clock Source", &tmp_measure_captures.sysclk_src_combo, "Internal\0External");

        ImGui::InputText("System Clock Frequency", tmp_measure_captures.sysclk_freq.data(), tmp_measure_captures.sysclk_freq.size());

        ImGui::Separator(); 

        ImGui::Button("Program Device Registers");
        ImGui::Button("Start Sweep");

        ImGui::Separator(); 

        ImGui::Text("REAL_DATA: 0xFFFF'FFFF");
        ImGui::Text("IMAG_DATA: 0xFFFF'FFFF");

        ImGui::End();
        tmp_measure_captures.update_config();
        measure_captures.store(std::make_shared<MeasurementWindowCaptures>(tmp_measure_captures));
    }
}

namespace ImGuiSDL {
    inline void imgui_create_window() {
        static ImVec2 measurement_window_size = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(measurement_window_size);
        measurement_window(std::ref(measurement_window_size));
        if(debug_window_enable.load()) {
            ImGui::SetNextWindowPos(ImVec2(measurement_window_size.x, 0.0f));
            ImGui::SetNextWindowSize(measurement_window_size);
            debug_registers_window();
        }
    }

    void loop() {
        SDL_Window* window;
        SDL_Renderer* renderer;
        {
            const auto ret = init();
            window = std::get<0>(ret);
            renderer = std::get<1>(ret);
        }

        bool done = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        while(esp32_ad5933.has_value() == false || esp32_ad5933.value().is_connected() == false) {
            process_events(window, &done);
            start_new_frame();
            imgui_create_connecting_window();
            render(renderer, clear_color);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
 
        done = false;
        while(done == false) {
            process_events(window, &done);
            start_new_frame();
            imgui_create_window();
            render(renderer, clear_color);
        }

        shutdown(renderer, window);
    }
}
