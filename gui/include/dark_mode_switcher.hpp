#pragma once

#include <iostream>

#include "SDL2/SDL_video.h"
#include "SDL2/SDL_syswm.h"
#include "imgui.h"

//#include <Windows.h>
//#include <winrt/Windows.UI.ViewManagement.h>
//#include <winrt/Windows.Foundation.h>

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
