#pragma once

#include <iostream>

#include "SDL3/SDL_video.h"
#include "imgui.h"

/*
class DarkModeSwitcher {
private:
    SDL_Window* window;
    bool dark_mode_active;
    //static inline bool IsColorDark(const winrt::Windows::UI::Color& clr);
public:
    DarkModeSwitcher(SDL_Window* in_window);
    //void setTitleBarDarkMode(const bool on_or_off);
    //static bool isGlobalDarkModeActive();
    //void enable_dynamic_switching();
};
*/

void switch_imgui_theme();