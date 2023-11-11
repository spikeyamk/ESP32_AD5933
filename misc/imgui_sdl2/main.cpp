#include <thread>

#include "SDL2/SDL_main.h"

#include "imgui_sdl.hpp"

int main(int argc, char* argv[]) {
    std::thread imgui_sdl_thread(imgui_sdl_thread_loop);
    imgui_sdl_thread.join();
    return 0;
}