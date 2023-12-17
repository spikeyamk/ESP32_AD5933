#pragma once

#include "imgui.h"

namespace GUI {
    namespace Spinner {
        bool BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col);
        bool Spinner(const char* label, float radius, float thickness, const ImU32& color);
    }
}