#include <iostream>
#include <chrono>
#include <functional>
#include <tuple>
#include <memory>

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

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

static const auto sdl_error_lambda = []() { std::cout << SDL_GetError() << std::endl; };

bool Imgui_CheckVersion() {
    return IMGUI_CHECKVERSION();
}

void imgui_sdl_shutdown(SDL_Renderer* renderer, SDL_Window* window) {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

inline void imgui_sdl_render(SDL_Renderer* renderer, ImVec4& clear_color) {
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

inline void imgui_sdl_process_events(SDL_Window* window, bool *done) {
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

inline void imgui_sdl_start_new_frame() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

inline void imgui_create_window(ImVec4 &clear_color) {
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
    {
        ImGui::Begin("Hello, world!", NULL, flags);                          // Create a window called "Hello, world!" and append into it.
        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        static int counter = 0;
        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        ImGui::End();
    }
}

std::tuple<SDL_Window*, SDL_Renderer*> imgui_sdl_init() {
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

    Trielo::trieloxit<Imgui_CheckVersion>(Trielo::OkErrCode(true));
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

void imgui_sdl_thread_loop() {
    SDL_Window* window;
    SDL_Renderer* renderer;
    {
        auto ret = imgui_sdl_init();
        window = std::get<0>(ret);
        renderer = std::get<1>(ret);
    }

    bool done = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    while(done == false) {
        imgui_sdl_process_events(window, &done);
        imgui_sdl_start_new_frame();
        imgui_create_window(clear_color);
        imgui_sdl_render(renderer, clear_color);
    }
    imgui_sdl_shutdown(renderer, window);
}