#pragma once

#include <tuple>
#include "imgui.h"

#include "SDL2/SDL.h"

namespace GUI {
    namespace Boilerplate {
        std::tuple<SDL_Window*, SDL_Renderer*> init();
        void process_events(SDL_Window* window, bool &done);
        void start_new_frame();
        void render(SDL_Renderer* renderer, const ImVec4& clear_color);
        void shutdown(SDL_Renderer* renderer, SDL_Window* window);
    }
}
