#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <boost/process.hpp>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include "ble_client/shm.hpp"

namespace GUI {
    class Console {
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
