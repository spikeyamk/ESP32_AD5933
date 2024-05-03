#pragma once

#include <queue>
#include <string>
#include <optional>

#include "imgui.h"

namespace GUI {
    namespace Windows {
        class PopupQueue {
        private:
            struct Popup {
                std::string name;
                std::string text;
            };
            std::optional<Popup> active_popup { std::nullopt };
            std::queue<Popup> p_queue;
        public:
            PopupQueue() = default;
            void activate_front();
            void push_back(const std::string& text);
            void show_active();
            bool empty() const;
            bool active() const;
        private:
            void deactivate();
        };
    }
}