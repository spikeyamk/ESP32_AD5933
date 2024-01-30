#include "imgui_internal.h"

#include "gui/windows/console.hpp"

namespace GUI {
    Console::Console(bool& enable, boost::process::ipstream& stdout_stream, boost::process::ipstream& stderr_stream) :
        enable { enable },
        stdout_stream{ stdout_stream },
        stderr_stream{ stderr_stream }
    {}

    void Console::log(const std::string& text) {
        lines.push_back(text);
    }

    void Console::draw() {
        if(enable == false) {
            return;
        }

        if(ImGui::Begin("Console", &enable) == false) {
            ImGui::End();
            return;
        }

        update_text();
        ImGui::InputTextMultiline("##source", &text, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly, nullptr, nullptr);

        /* This snippet scrolls to the bottom of the InputTextMultiline https://github.com/ocornut/imgui/issues/5484#issuecomment-1189989347 */
        ImGuiContext& g = *GImGui;
        const char* child_window_name = NULL;
        ImFormatStringToTempBuffer(&child_window_name, NULL, "%s/%s_%08X", g.CurrentWindow->Name, "##source", ImGui::GetID("##source"));
        ImGuiWindow* child_window = ImGui::FindWindowByName(child_window_name);
        ImGui::SetScrollY(child_window, child_window->ScrollMax.y);

        ImGui::End();
    }

    void Console::update_text() {
        for(const auto& e: lines) {
            if(e.empty() == false) {
                text.append(e);
            }
        }
        lines.clear();
    }
}
