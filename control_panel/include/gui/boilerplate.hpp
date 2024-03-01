#pragma once

#include <tuple>
#include <functional>

#include "imgui.h"
#include "SDL3/SDL.h"

#include "json/settings.hpp"

namespace GUI {
    namespace Boilerplate {
        extern const std::function<void()> sdl_error_lambda;
        std::tuple<SDL_Window*, SDL_Renderer*, ns::SettingsFile> init();
        void process_events(bool &done, SDL_Window* window, SDL_Renderer* renderer);
        void start_new_frame();
        void render(SDL_Renderer* renderer);
        void shutdown(SDL_Renderer* renderer, SDL_Window* window);
        float get_scale();
        extern bool respect_system_theme;
        extern bool respect_system_scale;
        constexpr float font_size_pixels_base { 13.0f };
        void set_scale(const float scale);
    }
}
