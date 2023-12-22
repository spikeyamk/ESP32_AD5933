#include "imgui.h"
#include "gui/spinner.hpp"

#include "gui/windows/connecting.hpp"

namespace GUI {
    namespace Windows {
        void create_connecting() {
            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
            static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
            ImGui::Begin("ESP32_AD5933", NULL, flags);
            ImGui::Text("Connecting");
            ImGui::SameLine();
            Spinner::Spinner("ConnectingSpinner", 5.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
            ImGui::End();
        }
    }
}
