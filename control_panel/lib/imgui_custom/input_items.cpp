#include "imgui_custom/input_items.hpp"

#include "imgui.h"
#include "imgui_internal.h"

namespace ImGui {
    bool InputTextValid(const char* label, char* buf, size_t buf_size, bool valid, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
        if(valid == false) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
            const bool result = ImGui::InputText(label, buf, buf_size, flags, callback, user_data);
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            return result;
        }
        return ImGui::InputText(label, buf, buf_size, flags, callback, user_data);
    }

    bool InputScalarWithCallback(const char* label, ImGuiDataType data_type, void* p_data, const void* p_step, const void* p_step_fast, const char* format, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        ImGuiStyle& style = g.Style;

        if (format == NULL)
            format = DataTypeGetInfo(data_type)->PrintFmt;

        char buf[64];
        DataTypeFormatString(buf, IM_ARRAYSIZE(buf), data_type, p_data, format);

        flags |= ImGuiInputTextFlags_AutoSelectAll | (ImGuiInputTextFlags)ImGuiInputTextFlags_NoMarkEdited; // We call MarkItemEdited() ourselves by comparing the actual data rather than the string.

        bool value_changed = false;
        if (p_step == NULL)
        {
            if (InputText(label, buf, IM_ARRAYSIZE(buf), flags, callback))
                value_changed = DataTypeApplyFromText(buf, data_type, p_data, format);
        }
        else
        {
            const float button_size = GetFrameHeight();

            BeginGroup(); // The only purpose of the group here is to allow the caller to query item data e.g. IsItemActive()
            PushID(label);
            SetNextItemWidth(ImMax(1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));
            if (InputText("", buf, IM_ARRAYSIZE(buf), flags, callback)) // PushId(label) + "" gives us the expected ID from outside point of view
                value_changed = DataTypeApplyFromText(buf, data_type, p_data, format);
            IMGUI_TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Inputable);

            // Step buttons
            const ImVec2 backup_frame_padding = style.FramePadding;
            style.FramePadding.x = style.FramePadding.y;
            ImGuiButtonFlags button_flags = ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups;
            if (flags & ImGuiInputTextFlags_ReadOnly)
                BeginDisabled();
            SameLine(0, style.ItemInnerSpacing.x);
            if (ButtonEx("-", ImVec2(button_size, button_size), button_flags))
            {
                DataTypeApplyOp(data_type, '-', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
                value_changed = true;
            }
            SameLine(0, style.ItemInnerSpacing.x);
            if (ButtonEx("+", ImVec2(button_size, button_size), button_flags))
            {
                DataTypeApplyOp(data_type, '+', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
                value_changed = true;
            }
            if (flags & ImGuiInputTextFlags_ReadOnly)
                EndDisabled();

            const char* label_end = FindRenderedTextEnd(label);
            if (label != label_end)
            {
                SameLine(0, style.ItemInnerSpacing.x);
                TextEx(label, label_end);
            }
            style.FramePadding = backup_frame_padding;

            PopID();
            EndGroup();
        }
        if (value_changed)
            MarkItemEdited(g.LastItemData.ID);

        return value_changed;
    }

    bool InputScalarWithCallbackTextReadOnly(const char* label, ImGuiDataType data_type, void* p_data, const void* p_step, const void* p_step_fast, const char* format, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        ImGuiStyle& style = g.Style;

        if (format == NULL)
            format = DataTypeGetInfo(data_type)->PrintFmt;

        char buf[64];
        DataTypeFormatString(buf, IM_ARRAYSIZE(buf), data_type, p_data, format);

        flags |= ImGuiInputTextFlags_AutoSelectAll | (ImGuiInputTextFlags)ImGuiInputTextFlags_NoMarkEdited; // We call MarkItemEdited() ourselves by comparing the actual data rather than the string.

        bool value_changed = false;
        if (p_step == NULL)
        {
            if (InputText(label, buf, IM_ARRAYSIZE(buf), flags | ImGuiInputTextFlags_ReadOnly, callback))
                value_changed = DataTypeApplyFromText(buf, data_type, p_data, format);
        }
        else
        {
            const float button_size = GetFrameHeight();

            BeginGroup(); // The only purpose of the group here is to allow the caller to query item data e.g. IsItemActive()
            PushID(label);
            SetNextItemWidth(ImMax(1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));
            if (InputText("", buf, IM_ARRAYSIZE(buf), flags | ImGuiInputTextFlags_ReadOnly, callback)) // PushId(label) + "" gives us the expected ID from outside point of view
                value_changed = DataTypeApplyFromText(buf, data_type, p_data, format);
            IMGUI_TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Inputable);

            // Step buttons
            const ImVec2 backup_frame_padding = style.FramePadding;
            style.FramePadding.x = style.FramePadding.y;
            ImGuiButtonFlags button_flags = ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups;
            if (flags & ImGuiInputTextFlags_ReadOnly)
                BeginDisabled();
            SameLine(0, style.ItemInnerSpacing.x);
            if (ButtonEx("-", ImVec2(button_size, button_size), button_flags))
            {
                DataTypeApplyOp(data_type, '-', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
                value_changed = true;
            }
            SameLine(0, style.ItemInnerSpacing.x);
            if (ButtonEx("+", ImVec2(button_size, button_size), button_flags))
            {
                DataTypeApplyOp(data_type, '+', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
                value_changed = true;
            }
            if (flags & ImGuiInputTextFlags_ReadOnly)
                EndDisabled();

            const char* label_end = FindRenderedTextEnd(label);
            if (label != label_end)
            {
                SameLine(0, style.ItemInnerSpacing.x);
                TextEx(label, label_end);
            }
            style.FramePadding = backup_frame_padding;

            PopID();
            EndGroup();
        }
        if (value_changed)
            MarkItemEdited(g.LastItemData.ID);

        return value_changed;
    }

    bool InputFloat(const char* label, float* v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback) {
        return ImGui::InputScalarWithCallback(label, ImGuiDataType_Float, (void*)v, (void*)(step > 0.0f ? &step : NULL), (void*)(step_fast > 0.0f ? &step_fast : NULL), format, flags, callback);
    }

    bool InputFloatValid(const char* label, float* v, bool valid, float step, float step_fast, const char* format, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback) {
        if(valid == false) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
            const bool result = ImGui::InputFloat(label, v, step, step_fast, format, flags, callback);
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            return result;
        }
        return ImGui::InputFloat(label, v, step, step_fast, format, flags, callback);
    }

    bool Input_uint32_t(const char* label, uint32_t* v, uint32_t step, uint32_t step_fast, ImGuiInputTextFlags flags) {
        // Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
        const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%u";
        return ImGui::InputScalar(label, ImGuiDataType_U32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
    }

    bool Input_uint32_t_WithCallback(const char* label, uint32_t* v, uint32_t step, uint32_t step_fast, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback) {
        const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%u";
        return ImGui::InputScalarWithCallback(label, ImGuiDataType_U32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags, callback);
    }

    bool Input_uint32_t_WithCallbackTextReadOnly(const char* label, uint32_t* v, uint32_t step, uint32_t step_fast, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback) {
        const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%u";
        return ImGui::InputScalarWithCallbackTextReadOnly(label, ImGuiDataType_U32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags, callback);
    }
}