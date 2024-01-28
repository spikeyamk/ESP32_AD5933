#include "imgui_internal.h"

#include "gui/windows/plots/measurement.hpp"

namespace GUI {
    namespace Windows {
        void measurement_plots(ImGuiID side_id, bool &enable, Client& client) {
            static char base[] = "Measurement Plots";
            char name[30];
            std::sprintf(name, "%s##%zu", base, client.index);

            static size_t first = 0;
            if(first == client.index) {
                ImGui::DockBuilderDockWindow(name, side_id);
                first++;
            }

            if(ImGui::Begin(name, &enable) == false) {
                ImGui::End();
                return;
            }
            ImGui::End();
        }
    }
}
