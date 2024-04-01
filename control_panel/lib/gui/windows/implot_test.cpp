#include <algorithm>
#include <cmath>

#include "imgui.h"

#include "gui/windows/implot_test.hpp"

namespace GUI {
    namespace Windows {
        void ImplotTest::draw(bool& enable) {
            if(enable == false) {
                return;
            }

            if(ImGui::Begin("ImplotTest") == false) {
                return;
            }

            if(ImGui::Button("Generate Additional data")) {
                xs.resize(xs.size() + 1024 * sizeof(decltype(xs)::value_type));
                ys.resize(ys.size() + 1024 * sizeof(decltype(ys)::value_type));
                std::generate(xs.begin(), xs.end(), [index = 0.0] mutable { return index++; });
                std::generate(ys.begin(), ys.end(), [index = 0.0] mutable { return std::sin(index++); });
            }

            if(implot_context == nullptr) {
                implot_context = ImPlot::CreateContext();
            }

            ImPlot::SetCurrentContext(implot_context);

            if(ImPlot::BeginPlot("Implot Test")) {
                ImPlot::PlotLine<double>("Implot Test", xs.data(), ys.data(), ys.size());
                ImPlot::EndPlot();
            }

            ImGui::End();
        }

        ImplotTest::~ImplotTest() {
            if(implot_context != nullptr) {
                ImPlot::SetCurrentContext(implot_context);
                ImPlot::DestroyContext(implot_context);
            }
        }
    }
}