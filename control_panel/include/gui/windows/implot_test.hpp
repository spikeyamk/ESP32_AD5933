#pragma once

#include <vector>

#include "implot.h"

namespace GUI {
    namespace Windows {
        class ImplotTest {
            std::vector<double> xs {};
            std::vector<double> ys {};
            ImPlotContext* implot_context { nullptr };
        public:
            ImplotTest() = default;
            void draw(bool &enable);
            ~ImplotTest();
        };
    }
}