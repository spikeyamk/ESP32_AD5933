#pragma once

#include "implot.h"

namespace ImPlot { 
    void SetupAxisLabel(ImAxis idx, const char* label);
    void SetupAxesLabels(const char* x_label, const char* y_label);
}