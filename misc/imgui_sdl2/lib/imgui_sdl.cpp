#include <iostream>
#include <sstream>
#include <chrono>
#include <functional>
#include <tuple>
#include <memory>
#include <optional>

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

#include "dark_mode_switcher.hpp"
#include "imgui_sdl.hpp"
#include "ble.hpp"
#include "ad5933_config.hpp"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

namespace ImGuiSDL {
    static const auto sdl_error_lambda = []() { std::cout << SDL_GetError() << std::endl; };

    bool check_version() {
        return IMGUI_CHECKVERSION();
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

    static const auto default_config = AD5933_Config::get_default();
    void debug_registers_window() {
        static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
        ImGui::Begin("Register Debug", NULL, flags);                          // Create a window called "Hello, world!" and append into it.

        ImGui::Button("Dump All Registers");
        ImGui::Text("Decoded Register Values");
        ImGui::Separator();

        std::ostringstream control_HB_command_oss;
        control_HB_command_oss << default_config.get_command();
        std::ostringstream range_oss;
        range_oss << default_config.get_voltage_range();
        std::ostringstream pga_gain_oss;
        pga_gain_oss << default_config.get_pga_gain();

        std::ostringstream reset_oss;
        reset_oss << default_config.get_reset();
        std::ostringstream sysclk_src_oss;
        sysclk_src_oss << default_config.get_sysclk_src();

        std::ostringstream freq_start_oss;
        freq_start_oss << default_config.get_start_freq();
        std::ostringstream freq_inc_oss;
        freq_inc_oss << default_config.get_inc_freq();
        std::ostringstream num_of_inc_oss;
        num_of_inc_oss << default_config.get_num_of_inc();
        std::ostringstream num_of_settling_time_cycles_oss;
        num_of_settling_time_cycles_oss << default_config.get_settling_time_cycles_number();
        std::ostringstream settling_time_cycles_multiplier_oss;
        //settling_time_cycles_multiplier_oss << default_config.get_settling_time_cycles_multiplier();

        //std::ostringstream reset_oss;
        //reset_oss << default_config.has();

        //std::ostringstream temp_data_oss;
        //temp_data_oss << default_config.read();
        //std::ostringstream real_data_oss;
        //imag_data_oss << default_config.read();
        //std::ostringstream imag_data_oss ;
        //imag_data_oss  << default_config.read();

        ImGui::Text("ControlHB:");
        ImGui::Text("\tControlHB Command: %s", control_HB_command_oss.str().data());
        ImGui::Text("\tExcitation Output Voltage Range: %s", range_oss.str().data());
        ImGui::Text("\tPGA Gain: %s", pga_gain_oss.str().data());
        ImGui::Separator();

        ImGui::Text("ControlLB:");
        ImGui::Text("\tReset: %s", reset_oss.str().data());
        ImGui::Text("\tSystem Clock Source: %s", sysclk_src_oss.str().data());
        ImGui::Separator();

        ImGui::Text("Start Frequency: %s", freq_start_oss.str().data());
        ImGui::Text("Frequency Increment: %s", freq_inc_oss.str().data());
        ImGui::Text("Number of Increments: %s", num_of_inc_oss.str().data());
        ImGui::Text("Number of Settling Time Cycles: %s", num_of_settling_time_cycles_oss.str().data());
        ImGui::Text("Settling Time Cycles Multiplier: %s", settling_time_cycles_multiplier_oss.str().data());
        ImGui::Separator();

        ImGui::Text("Status: ");
        ImGui::Separator();

        ImGui::Text("Temperature Data: ");
        ImGui::Text("Real Data: ");
        ImGui::Text("Imaginary Data: ");
        ImGui::Separator();

        int control_HB_capture = 0;
        int control_LB_capture = 0;
        int freq_start_capture[3] { 0, 0, 0};
        int freq_inc_capture[3] { 0, 0, 0 };
        int num_of_inc_capture[2] { 0, 0 };
        int num_of_settling_time_cycles_capture[2] { 0, 0 };

        int status_capture = 0;

        int temp_data_capture[2] { 0, 0 };
        int real_data_capture[2] { 0, 0 };
        int imag_data_capture[2] { 0, 0 };

        ImGui::Text("Read/Write Registers");
        ImGui::InputInt("ControlHB", &control_HB_capture, 0, 0, 0);
        ImGui::InputInt("ControlLB", &control_LB_capture, 0, 0, 0); 
        ImGui::InputInt3("FREQ_START", freq_start_capture); 
        ImGui::InputInt3("FREQ_INC", freq_inc_capture); 
        ImGui::InputInt2("NUM_OF_INC", num_of_inc_capture);
        ImGui::InputInt2("NUM_OF_SETTLING_TIME_CYCLES", num_of_settling_time_cycles_capture);

        ImGui::Button("Program Read/Write Registers");
        ImGui::Separator();

        ImGui::Text("Special Status Register");
        ImGui::InputInt("STATUS", &status_capture, 0, 0, ImGuiInputTextFlags_ReadOnly);

        ImGui::Text("Read-only Registers");
        ImGui::InputInt2("TEMP_DATA", temp_data_capture, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputInt2("REAL_DATA", real_data_capture, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputInt2("IMAG_DATA", imag_data_capture, ImGuiInputTextFlags_ReadOnly);

        ImGui::Text("Send Control Register Command Controls");
        ImGui::Button("Power-down mode"); ImGui::SameLine();
            ImGui::Button("Standby mode"); ImGui::SameLine();
            ImGui::Button("No operation");

        ImGui::Button("Measure temperature");
        ImGui::Button("Initialize with start frequency"); ImGui::SameLine();
            ImGui::Button("Start frequency sweep"); ImGui::SameLine();
            ImGui::Button("Increment frequency"); ImGui::SameLine();
            ImGui::Button("Repeat frequency"); ImGui::SameLine();

        ImGui::End();
    }

    inline void ble_hello_world_window() {
        static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
        ImGui::Begin("BLE Hello World", NULL, flags);                          // Create a window called "Hello, world!" and append into it.
        ImGui::Text("BLE Hello World Demo");
        static char tx_buf[20];
        ImGui::InputText("TX", tx_buf, sizeof(tx_buf) / sizeof(tx_buf[0]));
        if(ImGui::Button("Send") && esp32_ad5933.has_value()) {
            esp32_ad5933.value().send(std::string(tx_buf));
        }
        static char rx_buf[20];
        static std::string received_label {};
        if(esp32_ad5933.has_value()) {
            ImGui::InputText(
                "RX",
                esp32_ad5933.value().rx_payload.value_or("0xFFFF'FFFF").data(),
                esp32_ad5933.value().rx_payload.value_or("0xFFFF'FFFF").size(),
                ImGuiInputTextFlags_ReadOnly
            );
        }
        ImGui::End();
    }

    inline void imgui_create_window() {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        //periodic_temperature_measurement_demo_window();
        //debug_registers_window();
        ble_hello_world_window();
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

        DarkModeSwitcher dark_mode_switcher { window };
        if(DarkModeSwitcher::isGlobalDarkModeActive()) {
            ImGui::StyleColorsDark();
        } else {
            ImGui::StyleColorsLight();
        }
        //dark_mode_switcher.enable_dynamic_switching();
        //SDL_ShowWindow(window);
        SDL_MaximizeWindow(window);
        return std::tuple { window, renderer };
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
        while(done == false) {
            process_events(window, &done);
            start_new_frame();
            imgui_create_window();
            render(renderer, clear_color);
        }
        shutdown(renderer, window);
    }
}