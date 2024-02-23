#pragma once

#include <tuple>
#include <filesystem>
#include "imgui.h"

#include "SDL3/SDL.h"

namespace GUI {
    namespace Boilerplate {
        std::tuple<SDL_Window*, SDL_Renderer*> init(const std::filesystem::path& font_path);
        void process_events(bool &done, SDL_Window* window, SDL_Renderer* renderer, const std::filesystem::path& font_path);
        void start_new_frame();
        void render(SDL_Renderer* renderer, const ImVec4& clear_color);
        void shutdown(SDL_Renderer* renderer, SDL_Window* window);
        float get_scale();
        extern bool respect_system_theme;
        extern bool respect_system_scale;
        constexpr float font_size_pixels_base { 13.0f };
        void set_scale(const float scale, const std::filesystem::path& font_path);
        extern Uint32 event_user_scale_event_type;
    }
}
