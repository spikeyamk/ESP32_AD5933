#pragma once

#include "imgui.h"

namespace ImGui {
    int plus_minus_dot_char_filter_cb(ImGuiInputTextCallbackData* data);
    int minus_char_filter_cb(ImGuiInputTextCallbackData* data);
}