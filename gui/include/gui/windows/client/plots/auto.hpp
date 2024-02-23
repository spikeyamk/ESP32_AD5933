#pragma once

#include <vector>
#include <string>
#include <string_view>

#include "imgui.h"

#include "gui/windows/client/auto.hpp"

namespace GUI {
    namespace Windows {
        namespace Plots {
            class Auto {
            public:
                static constexpr std::u8string_view name_base { u8"Auto Plots##" };
            private:
                size_t index;
                std::u8string name { name_base };
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