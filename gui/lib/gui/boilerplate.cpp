#include "trielo/trielo.hpp"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "gui/dark_mode_switcher.hpp"

#include "gui/boilerplate.hpp"

namespace GUI {
    namespace Boilerplate {
        // ImGuiSDL utility boilerplate functions
        static const auto sdl_error_lambda = []() {
            std::cout << SDL_GetError() << std::endl;
        };

        std::tuple<SDL_Window*, SDL_Renderer*> init() {
            Trielo::trieloxit_lambda<SDL_Init>(Trielo::OkErrCode(0), sdl_error_lambda, SDL_INIT_VIDEO);
            Trielo::trielo_lambda<SDL_SetHint>(Trielo::OkErrCode(SDL_bool{SDL_TRUE}), sdl_error_lambda, SDL_HINT_IME_SHOW_UI, "1");

            static constexpr Uint32 window_flags = (
                SDL_WINDOW_RESIZABLE
                | SDL_WINDOW_HIGH_PIXEL_DENSITY
                | SDL_WINDOW_MAXIMIZED
                | SDL_WINDOW_HIDDEN
            );
            SDL_Window* window = Trielo::trieloxit_lambda<SDL_CreateWindow>(
                Trielo::FailErrCode(static_cast<SDL_Window*>(nullptr)),
                sdl_error_lambda,
                "AD5933",
                1280,
                720,
                window_flags
            );

            static constexpr Uint32 renderer_flags = (
		        SDL_RENDERER_ACCELERATED
                | SDL_RENDERER_PRESENTVSYNC
            );
            SDL_Renderer* renderer = Trielo::trieloxit_lambda<SDL_CreateRenderer>(
                Trielo::FailErrCode(static_cast<SDL_Renderer*>(nullptr)),
                sdl_error_lambda,
                window,
                nullptr,
                renderer_flags 
            );
            {
                SDL_RendererInfo info;
                Trielo::trielo_lambda<SDL_GetRendererInfo>(
                    Trielo::OkErrCode(0),
                    sdl_error_lambda,
                    renderer,
                    &info
                );
                std::printf("Current SDL_Renderer: %s\n", info.name);
            }

            Trielo::trieloxit<ImGui::DebugCheckVersionAndDataLayout>(
                Trielo::OkErrCode(true),
                IMGUI_VERSION,
                sizeof(ImGuiIO),
                sizeof(ImGuiStyle),
                sizeof(ImVec2),
                sizeof(ImVec4),
                sizeof(ImDrawVert),
                sizeof(ImDrawIdx)
            );
            Trielo::trieloxit<ImGui::CreateContext>(Trielo::FailErrCode(static_cast<ImGuiContext*>(nullptr)), static_cast<ImFontAtlas*>(nullptr));

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

            // Setup Platform/Renderer backends
            ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
            ImGui_ImplSDLRenderer3_Init(renderer);

            switch_imgui_theme();
            //DarkModeSwitcher dark_mode_switcher { window };
            //if(DarkModeSwitcher::isGlobalDarkModeActive()) {
            //    ImGui::StyleColorsDark();
            //} else {
            //    ImGui::StyleColorsLight();
            //}
            //Commented out because of a bug in a lambda capture of this and segfault on when darkModeRevoker is fired
            //dark_mode_switcher.enable_dynamic_switching();

            Trielo::trielo_lambda<SDL_ShowWindow>(Trielo::OkErrCode(0), sdl_error_lambda, window);
            return std::tuple { window, renderer };
        } 

        void process_events(SDL_Window* window, bool &done) {
            SDL_Event event;
            while(SDL_PollEvent(&event)) {
                ImGui_ImplSDL3_ProcessEvent(&event);
                switch(event.type) {
                    case SDL_EVENT_QUIT:
                        done = true;
                        return;
                    case SDL_EVENT_SYSTEM_THEME_CHANGED:
                        switch_imgui_theme();
                    break; 
                }
            }
        }

        void start_new_frame() {
            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
        }

        void render(SDL_Renderer* renderer, const ImVec4& clear_color) {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Render();
            if(SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y) != 0) {
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
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData());
            if(SDL_RenderPresent(renderer) != 0) {
                fmt::print(fmt::fg(fmt::color::yellow), "WARNING: SDL_RenderClear failed\n");
                sdl_error_lambda();
            }
        }

        void shutdown(SDL_Renderer* renderer, SDL_Window* window) {
            ImGui_ImplSDLRenderer3_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
        }
    }
}
