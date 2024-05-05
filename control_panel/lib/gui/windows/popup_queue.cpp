#include "gui/windows/popup_queue.hpp"

namespace GUI {
    namespace Windows {
        void PopupQueue::activate_front() {
            active_popup = p_queue.front();
            p_queue.pop();

            ImGui::OpenPopup(active_popup.value().title.c_str());
            // Always center this window when appearing
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }

        void PopupQueue::show_active() {
            if(ImGui::BeginPopupModal(active_popup.value().title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                (active_popup.value().func.value_or(default_window))();
                ImGui::EndPopup();
            }
        }

        void PopupQueue::push_back(const std::string& title, const std::string& text = std::string(), std::optional<std::function<void()>> func) {
            p_queue.push(
                Popup {
                    .title = title,
                    .text = text,
                    .func = func
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
            ImGui::CloseCurrentPopup();
            active_popup = std::nullopt;
        }
    }
}