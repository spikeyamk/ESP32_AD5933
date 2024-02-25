#include "imgui_custom/char_filters.hpp"

namespace ImGui {
    static const int discard = 1;
    static const int nodiscard = 0;
    int plus_minus_dot_char_filter_cb(ImGuiInputTextCallbackData* data) {
        switch(data->EventFlag) {
            case ImGuiInputTextFlags_CallbackCharFilter:
                const ImWchar tmp_event_char = data->EventChar;
                if(
                    tmp_event_char == '-'
                    || tmp_event_char == '+'
                    || tmp_event_char == '.'
                ) {
                    return discard;
                }
        }
        return nodiscard;
    }

    int minus_char_filter_cb(ImGuiInputTextCallbackData* data) {
        switch(data->EventFlag) {
            case ImGuiInputTextFlags_CallbackCharFilter:
                const ImWchar tmp_event_char = data->EventChar;
                if(tmp_event_char == '-') {
                    return discard;
                }
        }
        return nodiscard;
    }
}