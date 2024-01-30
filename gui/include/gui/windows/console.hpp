#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <boost/process.hpp>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <ble_client/standalone/shm.hpp>

namespace GUI {
    class Console {
    private:
        std::vector<std::string> lines;
        std::string text;
        bool auto_scroll = true;
        bool& enable;
        boost::process::ipstream& stdout_stream;
        boost::process::ipstream& stderr_stream;
        std::mutex mutex;
    public:
        Console(bool& enable, boost::process::ipstream& stdout_stream, boost::process::ipstream& stderr_stream);
        void log(const std::string& text);
        void draw();
    private:
        inline void update_text();
    };
}
