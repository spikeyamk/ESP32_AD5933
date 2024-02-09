#pragma once

#include <vector>
#include <string>

#include "imgui.h"

namespace GUI {
    namespace Windows {
        namespace Plots {
            class Auto {
                size_t index;
                std::string name { "Auto Plots##" };
                bool first { true };
            public:
                Auto() = default;
                Auto(const size_t index);
                void draw(bool& enable, const ImGuiID side_id);
            private:
                void draw_corrected_gon_data();
                void draw_corrected_alg_data();
            };
        }
    }
}