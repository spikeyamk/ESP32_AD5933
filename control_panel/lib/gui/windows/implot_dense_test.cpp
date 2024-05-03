#pragma once

#include <algorithm>
#include <ranges>
#include <cmath>

#include <imgui.h>
#include <implot.h>

#include "gui/windows/implot_dense_test.hpp"

const std::array<float, ImPlotDenseTest::xs_ys_size> ImPlotDenseTest::xs {
    []() {
        std::array<float, ImPlotDenseTest::xs_ys_size> ret;
        std::ranges::generate(ret.begin(), ret.end(), [n = 0.0f]() mutable { return n++; });
        return ret;
    }()
};

const std::array<float, ImPlotDenseTest::xs_ys_size> ImPlotDenseTest::ys {
    []() {
        std::array<float, ImPlotDenseTest::xs_ys_size> ret;
        std::ranges::generate(ret.begin(), ret.end(), [n = 0.0f]() mutable { return std::sin(n++); });
        return ret;
    }()
};

void ImPlotDenseTest::draw(bool& enable) {
    if(enable == false) {
        return;
    }

    if(ImGui::Begin("ImPlot Dense Test", &enable) == false) {
        ImGui::End();
        return;
    }

    ImPlot::BeginPlot("ImPlot Dense Test");
    ImPlot::PlotLine("ImPlot Dense Plot", xs.data(), ys.data(), std::min(xs.size(), ys.size()));
    ImPlot::EndPlot();

    ImGui::End();
}