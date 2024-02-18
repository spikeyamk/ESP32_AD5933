#pragma once

#include <vector>
#include <string>

#include "imgui.h"

#include "gui/windows/auto.hpp"

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
                void update_send_vectors(std::queue<Windows::Auto::Point>& send_points);
            private:
                struct Vectors {
                    std::vector<double> time;
                    std::vector<double> impedance;
                    std::vector<double> phase;
                    std::vector<double> resistance;
                    std::vector<double> reactance;
                };
                Vectors send_vectors {};
                void draw_send_corrected_gon_data();
                void draw_send_corrected_alg_data();
            };
        }
    }
}