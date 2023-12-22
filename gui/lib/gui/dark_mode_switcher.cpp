#include <cassert>
#include "SDL3/SDL_video.h"

#include "gui/dark_mode_switcher.hpp"

void switch_imgui_theme() {
    switch(SDL_GetSystemTheme()) {
        case SDL_SYSTEM_THEME_DARK:
            ImGui::StyleColorsDark();
            break;
        default:
            ImGui::StyleColorsLight();
            break;
    }
}

/*
DarkModeSwitcher::DarkModeSwitcher(SDL_Window* in_window) : 
    window {in_window} 
{
    assert(in_window != nullptr);
    if(isGlobalDarkModeActive()) {
        //setTitleBarDarkMode(true);
        dark_mode_active = true;
    } else {
        //setTitleBarDarkMode(false);
        dark_mode_active = false;
    }
}
*/

/*
//https://github.com/libsdl-org/SDL/issues/4776#issuecomment-926976455
void DarkModeSwitcher::setTitleBarDarkMode(const bool on_or_off) {
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    SDL_GetWindowWMInfo(window, &wmi);
    auto hwnd = wmi.info.win.window;

    auto uxtheme = LoadLibraryA("uxtheme.dll");
    auto dwm = LoadLibraryA("dwmapi.dll");

    if (uxtheme && dwm) {
        typedef HRESULT (*SetWindowThemePTR)(HWND, const wchar_t *, const wchar_t *);
        SetWindowThemePTR SetWindowTheme = (SetWindowThemePTR)GetProcAddress(uxtheme, "SetWindowTheme");

        typedef HRESULT (*DwmSetWindowAttributePTR)(HWND, DWORD, LPCVOID, DWORD);
        DwmSetWindowAttributePTR DwmSetWindowAttribute = (DwmSetWindowAttributePTR)GetProcAddress(dwm, "DwmSetWindowAttribute");

        if (SetWindowTheme && DwmSetWindowAttribute) {
            SetWindowTheme(hwnd, L"DarkMode_Explorer", NULL);

            BOOL darkMode = on_or_off == true ? 1 : 0;
            if (!DwmSetWindowAttribute(hwnd, 20, &darkMode, sizeof darkMode)) {
                DwmSetWindowAttribute(hwnd, 19, &darkMode, sizeof darkMode);
            }
        }
    }

    // Paints the background of the window black
    PAINTSTRUCT ps;
    RECT rc;
    HDC hdc = BeginPaint(hwnd, &ps);
    GetClientRect(hwnd, &rc);
    SetBkColor(hdc, BLACK_BRUSH);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, 0, 0, 0);
    EndPaint(hwnd, &ps);
    FreeLibrary(dwm);
    FreeLibrary(uxtheme);
}

//https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/apply-windows-themes
bool DarkModeSwitcher::IsColorDark(const winrt::Windows::UI::Color& clr) {
    return (((5 * clr.G) + (2 * clr.R) + clr.B) > (8 * 128));
}

bool DarkModeSwitcher::isGlobalDarkModeActive() {
    const auto settings = winrt::Windows::UI::ViewManagement::UISettings();
    const auto foreground = settings.GetColorValue(winrt::Windows::UI::ViewManagement::UIColorType::Foreground);
    return IsColorDark(foreground);
}

//https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/apply-windows-themes
void DarkModeSwitcher::enable_dynamic_switching() {
    auto settings = winrt::Windows::UI::ViewManagement::UISettings();
    auto revoker = settings.ColorValuesChanged([this, settings](auto&&...) {
        const bool isDarkModeRevoker = isGlobalDarkModeActive(); 
        std::cout << "isDarkModeRevoker: " << isDarkModeRevoker << std::endl;
        if (isDarkModeRevoker == true) {
            this->setTitleBarDarkMode(true);
            ImGui::StyleColorsDark();
            this->dark_mode_active = true;
        } else {
            this->setTitleBarDarkMode(false);
            ImGui::StyleColorsLight();
            this->dark_mode_active = false;
        }
    });
}
*/
