#pragma once

#include <tuple>
#include "imgui.h"

#include "SDL3/SDL.h"

namespace GUI {
    namespace Boilerplate {
        void file_open();
        std::tuple<SDL_Window*, SDL_Renderer*> init();
        void process_events(bool &done);
        void start_new_frame();
        void render(SDL_Renderer* renderer, const ImVec4& clear_color);
        void shutdown(SDL_Renderer* renderer, SDL_Window* window);
    }
}
