#include "imgui_internal.h"

#include "gui/windows/file_manager.hpp"

namespace GUI {
    namespace Windows {
        void file_manager(int i, ImGuiID side_id, bool &enable, Client &client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
            static char base[] = "File Manager";
            char name[20];
            std::sprintf(name, "%s##%d", base, i);

            static int first = 0;
            if(first == i) {
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
