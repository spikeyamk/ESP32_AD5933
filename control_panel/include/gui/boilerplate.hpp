#pragma once

#include <tuple>
#include <functional>

#include <imgui.h>
#include <SDL3/SDL.h>

#include "json/settings.hpp"

namespace GUI {
    namespace Boilerplate {
        extern const std::function<void()> sdl_error_lambda;
        extern bool respect_system_theme;
        extern bool respect_system_scale;
        extern const float font_size_pixels_base;

        std::tuple<SDL_Window*, SDL_Renderer*, ns::SettingsFile> init();
        void process_events(SDL_Window* window, SDL_Renderer* renderer, bool& out_sdl_event_quit);
        void start_new_frame();
        void render_skip_frame(SDL_Renderer* renderer);
        void render(SDL_Renderer* renderer);
        void shutdown(SDL_Renderer* renderer, SDL_Window* window);
        float get_scale();
    }
}
