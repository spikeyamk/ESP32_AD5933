#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include "ble_client/shm/parent/parent.hpp"

namespace GUI {
    namespace Windows {
        class Console {
        public:
            static constexpr std::u8string_view name { u8"Console" };
        private:
            std::vector<std::string> lines;
            std::string text;
            bool auto_scroll = true;
            bool& enable;
            std::mutex mutex;
        public:
            Console(bool& enable);
            void log(const std::string& text);
            void draw();
        private:
            inline void update_text();
        };
    }
}
