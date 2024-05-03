#include "gui/boilerplate.hpp"

#include "gui/windows/popup_queue.hpp"

namespace GUI {
    namespace Windows {
        void PopupQueue::activate_front() {
            active_popup = p_queue.front();
            p_queue.pop();

            ImGui::OpenPopup(active_popup.value().name.c_str());
            // Always center this window when appearing
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }

        void PopupQueue::show_active() {
            if(ImGui::BeginPopupModal(active_popup.value().name.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text(active_popup.value().text.c_str());
                if(ImGui::Button("OK", ImVec2(64 * GUI::Boilerplate::get_scale(), 0.0f))) {
                    ImGui::CloseCurrentPopup();
                    deactivate();
                }
                ImGui::EndPopup();
            }
        }

        void PopupQueue::push_back(const std::string& text) {
            p_queue.push(
                Popup {
                    .name = std::string("Error"),
                    .text = text
                }
            );
        }

        bool PopupQueue::empty() const {
            return p_queue.empty();
        }

        bool PopupQueue::active() const {
            return active_popup.has_value();
        }

        void PopupQueue::deactivate() {
            active_popup = std::nullopt;
        }
    }
}