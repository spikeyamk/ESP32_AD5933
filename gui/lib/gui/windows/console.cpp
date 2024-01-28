#include "imgui_internal.h"

#include "gui/windows/console.hpp"

namespace GUI {
    namespace Windows {
        void console(
            bool &enable,
            ImGuiID bottom_id
        ) {
            if(enable == false) {
                return;
            }

            /*
            static bool first = true;
            if(first) {
                ImGui::DockBuilderDockWindow("Console", bottom_id);
                first = false;
            }
            */

            if(ImGui::Begin("Console", &enable) == false) {
                ImGui::End();
                return;
            }

            ImGui::End();
        }
    }
}
