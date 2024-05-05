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
                struct Firsts {
                    struct Inner {
                        bool corrected_gon { true };
                        bool corrected_alg { true };
                    };
                    Inner send {};
                    Inner save {};
                };
                Firsts firsts {};
            public:
                Auto() = default;
                Auto(const size_t index);
                void draw(bool& enable, const ImGuiID side_id);
                void update_send_vectors(const Windows::Auto::Point& send_point);
                void clear_save_vectors();
                void update_save_vectors(const Windows::Auto::Point& save_point);
            private:
                struct Vectors {
                    std::vector<double> time;
                    std::vector<double> impedance;
                    std::vector<double> phase;
                    std::vector<double> resistance;
                    std::vector<double> reactance;
                };
                Vectors send_vectors {};
                Vectors save_vectors {};
                void update_vectors(const Windows::Auto::Point& point, Vectors& vectors);
                void draw_send_corrected_gon_data();
                void draw_send_corrected_alg_data();
                void draw_save_corrected_gon_data();
                void draw_save_corrected_alg_data();
            };
        }
    }
}