#include "imgui_internal.h"

#include "gui/windows/auto.hpp"

namespace GUI {
    namespace Windows {
        Auto::Auto(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) :
            index { index },
            shm { shm }
        {
            name.append(std::to_string(index));
        }

        void Auto::draw(bool &enable, const ImGuiID side_id) {
            if(first) {
                ImGui::DockBuilderDockWindow(name.c_str(), side_id);
                first = false;
            }

            if(ImGui::Begin(name.c_str(), &enable) == false) {
                ImGui::End();
                return;
            }

            ImGui::End();
        }

        void Auto::start_saving() {

        }

        void Auto::start_sending() {

        }

        void Auto::stop() {

        }

        void Auto::stop_saving() {

        }

        void Auto::stop_sending() {

        }
    }
}