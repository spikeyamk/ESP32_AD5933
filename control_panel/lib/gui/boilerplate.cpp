#include <span>
#include <fstream>

#include <trielo/trielo.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <nfd.hpp>
#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "gui/ubuntu_sans_regular.hpp"

#include "gui/boilerplate.hpp"

namespace GUI {
    namespace Boilerplate {
        const std::function<void()> sdl_error_lambda {
            []() {
                std::cout << SDL_GetError() << std::endl;
            }
        };

        bool respect_system_theme { true };
        inline void switch_imgui_theme() {
            if(respect_system_theme == false) {
                return;
            }

            switch(Trielo::trielo<SDL_GetSystemTheme>()) {
                case SDL_SYSTEM_THEME_DARK:
                    ImGui::StyleColorsDark();
                    break;
                default:
                    ImGui::StyleColorsLight();
                    break;
            }
        }

        void set_scale(const float scale) {
            ImGui_ImplSDLRenderer3_DestroyFontsTexture();
            ImGui::GetIO().Fonts->Clear();
            static const ImWchar font_ranges_magic_load_all_utf8_glyphs[] {
                0x20, 0xFFFF, 0 
            };

            uint8_t* font = new uint8_t[ubuntu_sans_regular.size()]; // This is not a memory leak io.Fonts->AddFontFromMemoryTTF calls delete on this variable
            std::copy(ubuntu_sans_regular.begin(), ubuntu_sans_regular.end(), font);
            ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font, static_cast<int>(ubuntu_sans_regular.size()), font_size_pixels_base * scale, nullptr, font_ranges_magic_load_all_utf8_glyphs);

            decltype(ImGuiStyle::Colors) original_colors {};
            for(size_t i = 0; i < sizeof(original_colors) / sizeof(original_colors[0]); i++) {
                original_colors[i] = ImGui::GetStyle().Colors[i];
            }

            ImGui::GetStyle() = ImGuiStyle();

            for(size_t i = 0; i < sizeof(original_colors) / sizeof(original_colors[0]); i++) {
                ImGui::GetStyle().Colors[i] = original_colors[i];
            }

            ImGui::GetStyle().ScaleAllSizes(scale);
        }

        float get_scale() {
            return ImGui::GetIO().Fonts->ConfigData[0].SizePixels / font_size_pixels_base;
        }

        bool respect_system_scale { true };
        void set_sdl_scale(const float sdl_scale) {
            if(respect_system_scale == false) {
                return;
            }

            set_scale(sdl_scale);
        }

        std::tuple<SDL_Window*, SDL_Renderer*, ns::SettingsFile> init() {
            ns::SettingsFile settings_file;
            if(Trielo::trielo_lambda<SDL_Init>(Trielo::OkErrCode(0), sdl_error_lambda, SDL_INIT_VIDEO) != 0) {
                return { nullptr, nullptr, settings_file };
            }
            Trielo::trielo_lambda<SDL_SetHint>(Trielo::OkErrCode(SDL_bool{SDL_TRUE}), sdl_error_lambda, SDL_HINT_IME_SHOW_UI, "1");

            Trielo::trielo<NFD::Init>(Trielo::OkErrCode(NFD_OKAY));
            Trielo::trielo<SDL_RegisterEvents>(Trielo::OkErrCode(static_cast<Uint32>(SDL_EVENT_USER)), 1);

            static constexpr Uint32 window_flags = (
                SDL_WINDOW_RESIZABLE
                | SDL_WINDOW_HIGH_PIXEL_DENSITY
                | SDL_WINDOW_MAXIMIZED
                | SDL_WINDOW_HIDDEN
            );
            SDL_Window* window { nullptr };
            if(
                (
                    window = Trielo::trielo_lambda<SDL_CreateWindow>(
                        Trielo::FailErrCode(static_cast<SDL_Window*>(nullptr)),
                        sdl_error_lambda,
                        "AD5933",
                        1280,
                        720,
                        window_flags
                    )
                )
            == nullptr) {
                return { nullptr, nullptr, settings_file };
            }

            static const Uint32 renderer_flags = (
		        SDL_RENDERER_ACCELERATED
                | SDL_RENDERER_PRESENTVSYNC
            );

            SDL_Renderer* renderer { nullptr };
            if(
                (
                    renderer = Trielo::trielo_lambda<SDL_CreateRenderer>(
                        Trielo::FailErrCode(static_cast<SDL_Renderer*>(nullptr)),
                        sdl_error_lambda,
                        window,
                        nullptr,
                        renderer_flags 
                    )
                ) 
            == nullptr) {
                return { nullptr, nullptr, settings_file };
            }

            {
                SDL_RendererInfo info;
                if(Trielo::trielo_lambda<SDL_GetRendererInfo>(
                    Trielo::OkErrCode(0),
                    sdl_error_lambda,
                    renderer,
                    &info
                ) == 0) {
                    std::printf("GUI::Boilerplate::init: Current SDL_Renderer: %s\n", info.name);
                }
            }

            if(Trielo::trielo<ImGui::DebugCheckVersionAndDataLayout>(
                Trielo::OkErrCode(true),
                IMGUI_VERSION,
                sizeof(ImGuiIO),
                sizeof(ImGuiStyle),
                sizeof(ImVec2),
                sizeof(ImVec4),
                sizeof(ImDrawVert),
                sizeof(ImDrawIdx)
            ) == false) {
                return { nullptr, nullptr, settings_file };
            }
            if(Trielo::trielo<ImGui::CreateContext>(Trielo::FailErrCode(static_cast<ImGuiContext*>(nullptr)), static_cast<ImFontAtlas*>(nullptr)) == nullptr) {
                return { nullptr, nullptr, settings_file };
            }
            if(Trielo::trielo<ImPlot::CreateContext>(Trielo::FailErrCode(static_cast<ImPlotContext*>(nullptr))) == nullptr) {
                return { nullptr, nullptr, settings_file };
            }

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
            io.IniFilename = nullptr;

            if(Trielo::trielo<ImGui_ImplSDL3_InitForSDLRenderer>(Trielo::OkErrCode(true), window, renderer) == false) {
                return { nullptr, nullptr, settings_file };
            }

            if(Trielo::trielo<ImGui_ImplSDLRenderer3_Init>(Trielo::OkErrCode(true), renderer) == false) {
                return { nullptr, nullptr, settings_file };
            }
            
            try {
                std::ifstream file(ns::settings_file_path);
                if(file.is_open() == false) {
                    throw std::exception("ns::settings_file_path doesn't exist");
                }

                json j;
                file >> j;
                settings_file = j;
                respect_system_theme = (settings_file.settings.theme_combo == 0);
                respect_system_scale = (settings_file.settings.scale_combo == 0);

                switch(settings_file.settings.theme_combo) {
                    case 0:
                        switch_imgui_theme();
                        break;
                    case 1:
                        ImGui::StyleColorsDark();
                        break;
                    case 2:
                        ImGui::StyleColorsLight();
                        break;
                    case 3:
                        ImGui::StyleColorsClassic();
                        break;
                }

                if(settings_file.settings.scale_combo == 0) {
                    Trielo::trielo<set_scale>(Trielo::trielo_lambda<SDL_GetWindowDisplayScale>(Trielo::OkErrCode(0.0f), Boilerplate::sdl_error_lambda, (window)));
                } else {
                    Trielo::trielo<set_scale>(ns::Settings::Scales::values[static_cast<size_t>(settings_file.settings.scale_combo - 1)]);
                }

                Trielo::trielo<set_implot_scale>();

                ImPlot::GetStyle().UseLocalTime = settings_file.settings.local_time;
                ImPlot::GetStyle().UseISO8601 = settings_file.settings.iso_8601;
                ImPlot::GetStyle().Use24HourClock = settings_file.settings.twenty_four_hour_clock;
            } catch(const std::exception& e) {
                std::cout << "ERROR: GUI::Top::Top(): error loading " << ns::settings_file_path << " file and parsing to json: exception: " << e.what() << std::endl;
                switch_imgui_theme();
                Trielo::trielo<set_scale>(Trielo::trielo_lambda<SDL_GetWindowDisplayScale>(Trielo::OkErrCode(0.0f), Boilerplate::sdl_error_lambda, (window)));
                Trielo::trielo<set_implot_scale>();
            }

            if(Trielo::trielo_lambda<SDL_ShowWindow>(Trielo::OkErrCode(0), Boilerplate::sdl_error_lambda, window) != 0) {
                return { nullptr, nullptr, settings_file };
            }

            return std::tuple { window, renderer, settings_file };
        } 

        void set_implot_scale() {
            const auto default_style { ImPlotStyle() };
            const float scale { Boilerplate::get_scale() };
            ImPlot::GetStyle().PlotDefaultSize.x = default_style.PlotDefaultSize.x * scale;
            ImPlot::GetStyle().PlotDefaultSize.y = default_style.PlotDefaultSize.y * scale;
        }

        void process_events(bool &done, SDL_Window* window, SDL_Renderer* renderer) {
            SDL_Event event;
            while(SDL_PollEvent(&event)) {
                ImGui_ImplSDL3_ProcessEvent(&event);
                switch(event.type) {
                    case SDL_EVENT_QUIT:
                        done = true;
                        return;
                   case SDL_EVENT_SYSTEM_THEME_CHANGED:
                        Trielo::trielo<switch_imgui_theme>();
                        break; 
                    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
                        // This fires when changes system wide scale settings change
                        Trielo::trielo<set_sdl_scale>(Trielo::trielo_lambda<SDL_GetWindowDisplayScale>(Trielo::OkErrCode(0.0f), sdl_error_lambda, window));
                        Trielo::trielo<set_implot_scale>();
                        break;
                    case SDL_EVENT_USER:
                        // This fires if user explicitally changes scale ignoring system wide settings
                        Trielo::trielo<set_scale>(*reinterpret_cast<const float*>(&event.user.code));
                        Trielo::trielo<set_implot_scale>();
                        break;
                }
            }
        }

        void start_new_frame() {
            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
        }

        void render(SDL_Renderer* renderer) {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Render();
            if(SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y) != 0) {
                fmt::print(fmt::fg(fmt::color::yellow), "WARNING: SDL_RenderSetScale failed");
                sdl_error_lambda();
            }
            static constexpr ImVec4 clear_color { 0.45f, 0.55f, 0.60f, 1.00f };
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
