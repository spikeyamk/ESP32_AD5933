#include <trielo/trielo.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <nfd.hpp>
#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"

#include "gui/boilerplate.hpp"

namespace GUI {
    namespace Boilerplate {
        static const auto sdl_error_lambda = []() {
            std::cout << SDL_GetError() << std::endl;
        };

        bool respect_system_theme { true };
        Uint32 event_user_scale_event_type { (Uint32)(-1) };
        inline void switch_imgui_theme() {
            if(respect_system_theme == false) {
                return;
            }

            switch(SDL_GetSystemTheme()) {
                case SDL_SYSTEM_THEME_DARK:
                    ImGui::StyleColorsDark();
                    break;
                default:
                    ImGui::StyleColorsLight();
                    break;
            }
        }

        void set_scale(const float scale, const std::filesystem::path& font_path) {
            #ifdef _MSC_VER // I hate Windows, also fuck Bill Gates
                ImGui_ImplSDLRenderer3_DestroyFontsTexture();
                ImGuiIO& io { ImGui::GetIO() };
                io.Fonts->Clear();
                static constexpr ImWchar font_ranges_magic_load_all_utf8_glyphs[] {
                    0x20, 0xFFFF, 0 
                };
                io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), font_size_pixels_base * scale, nullptr, font_ranges_magic_load_all_utf8_glyphs);
                ImGuiStyle& style = ImGui::GetStyle();
                style = ImGuiStyle();
                style.ScaleAllSizes(scale);
            #endif
        }

        float get_scale() {
            return ImGui::GetIO().Fonts->ConfigData[0].SizePixels / font_size_pixels_base;
        }

        bool respect_system_scale { true };
        static inline void set_sdl_scale(const float sdl_scale, const std::filesystem::path& font_path) {
            if(respect_system_scale == false) {
                return;
            }

            set_scale(sdl_scale, font_path);
        }

        std::tuple<SDL_Window*, SDL_Renderer*> init(const std::filesystem::path& font_path) {
            Trielo::trieloxit_lambda<SDL_Init>(Trielo::OkErrCode(0), sdl_error_lambda, SDL_INIT_VIDEO);
            Trielo::trielo_lambda<SDL_SetHint>(Trielo::OkErrCode(SDL_bool{SDL_TRUE}), sdl_error_lambda, SDL_HINT_IME_SHOW_UI, "1");

            Trielo::trielo<NFD::Init>(Trielo::OkErrCode(NFD_OKAY));
            event_user_scale_event_type = SDL_RegisterEvents(1);

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

            Trielo::trielo_lambda<SDL_SetHint>(Trielo::OkErrCode(SDL_bool{SDL_TRUE}), sdl_error_lambda, SDL_HINT_VIDEO_X11_SCALING_FACTOR, std::to_string(SDL_GetWindowDisplayScale(window)).c_str());

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
            Trielo::trieloxit<ImPlot::CreateContext>(Trielo::FailErrCode(static_cast<ImPlotContext*>(nullptr)));

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;

            ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
            ImGui_ImplSDLRenderer3_Init(renderer);

            switch_imgui_theme();

            Trielo::trielo_lambda<SDL_ShowWindow>(Trielo::OkErrCode(0), sdl_error_lambda, window);
            return std::tuple { window, renderer };
        } 

        void set_implot_scale() {
            const float scale = Boilerplate::get_scale();
            const auto default_style { ImPlotStyle() };
            ImPlot::GetStyle().PlotDefaultSize.x = default_style.PlotDefaultSize.x * scale;
            ImPlot::GetStyle().PlotDefaultSize.y = default_style.PlotDefaultSize.y * scale;
        }

        void process_events(bool &done, SDL_Window* window, SDL_Renderer* renderer, const std::filesystem::path& font_path) {
            SDL_Event event;
            if(SDL_PollEvent(&event)) {
                ImGui_ImplSDL3_ProcessEvent(&event);
                switch(event.type) {
                    case SDL_EVENT_QUIT:
                        done = true;
                        return;
                    case SDL_EVENT_SYSTEM_THEME_CHANGED:
                        switch_imgui_theme();
                        break; 
                    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
                        set_sdl_scale(SDL_GetWindowDisplayScale(window), font_path);
                        set_implot_scale();
                        break; 
                }
                if(event.type == event_user_scale_event_type) {
                    set_scale(*reinterpret_cast<const float*>(&event.user.code), font_path);
                    set_implot_scale();
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
            NFD::Quit();
            SDL_Quit();
        }
    }
}
