#pragma once

#include <cstddef>
#include <cstdint>

#include "imgui.h"

namespace ImGui {
    bool InputTextValid(const char* label, char* buf, size_t buf_size, bool valid, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    bool InputScalarWithCallback(const char* label, ImGuiDataType data_type, void* p_data, const void* p_step, const void* p_step_fast, const char* format, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback = NULL);
    bool InputFloat(const char* label, float* v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback = NULL);
    bool InputFloatValid(const char* label, float* v, bool valid, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL);
    bool Input_uint32_t(const char* label, uint32_t* v, uint32_t step, uint32_t step_fast, ImGuiInputTextFlags flags = 0);
    bool Input_uint32_t_WithCallback(const char* label, uint32_t* v, uint32_t step, uint32_t step_fast, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback);
    bool Input_uint32_t_WithCallbackTextReadOnly(const char* label, uint32_t* v, uint32_t step, uint32_t step_fast, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback);
}