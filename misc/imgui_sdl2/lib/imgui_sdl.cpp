#include <iostream>
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
                esp32_ad5933.value().subscribe_to_body_composition_measurement();
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

    inline void imgui_create_window(ImVec4 &clear_color) {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        periodic_temperature_measurement_demo_window();
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
            imgui_create_window(clear_color);
            render(renderer, clear_color);
        }
        shutdown(renderer, window);
    }
}