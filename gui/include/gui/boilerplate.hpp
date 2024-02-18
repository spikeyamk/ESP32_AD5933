#pragma once

#include <tuple>
#include "imgui.h"

#include "SDL3/SDL.h"

namespace GUI {
    namespace Boilerplate {
        std::tuple<SDL_Window*, SDL_Renderer*> init();
        void process_events(bool &done, SDL_Window* window, SDL_Renderer* renderer);
        void process_events(bool &done, SDL_Window* window);
        void start_new_frame();
        void render(SDL_Renderer* renderer, const ImVec4& clear_color);
        void shutdown(SDL_Renderer* renderer, SDL_Window* window);
        float get_scale();
    }
}
