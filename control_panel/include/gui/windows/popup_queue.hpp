#pragma once

#include <queue>
#include <string>
#include <optional>
#include <functional>

#include "imgui.h"
#include "gui/boilerplate.hpp"

namespace GUI {
    namespace Windows {
        class PopupQueue {
        private:
            struct Popup {
                std::string title;
                std::string text;
                std::optional<std::function<void()>> func;
            };
            std::optional<Popup> active_popup { std::nullopt };
            std::queue<Popup> p_queue;
            const std::function<void()> default_window { [&]() {
                if(active_popup.value().text.empty() == false) {
                    ImGui::Text(active_popup.value().text.c_str());
                }

                if(ImGui::Button("OK", ImVec2(64 * GUI::Boilerplate::get_scale(), 0.0f))) {
                    deactivate();
                }
            } };
        public:
            PopupQueue() = default;
            void activate_front();
            void push_back(const std::string& title, const std::string& text, std::optional<std::function<void()>> func = std::nullopt);
            void show_active();
            bool empty() const;
            bool active() const;
            void deactivate();
        };
    }
}