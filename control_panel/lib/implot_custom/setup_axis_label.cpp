#include "implot_custom/setup_axis_label.hpp"
#include "implot_internal.h"

namespace ImPlot {
    void SetupAxisLabel(ImAxis idx, const char* label) {
        ImPlotContext& gp = *GImPlot;
        IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr && !gp.CurrentPlot->SetupLocked,
                            "Setup needs to be called after BeginPlot and before any setup locking functions (e.g. PlotX)!");
        // get plot and axis
        ImPlotPlot& plot = *gp.CurrentPlot;
        ImPlotAxis& axis = plot.Axes[idx];
        // set ID
        axis.ID = plot.ID + idx + 1;
        // enable axis
        axis.Enabled = true;
        // set label
        plot.SetAxisLabel(axis, label);
    }
    
    void SetupAxesLabels(const char* x_label, const char* y_label) {
        SetupAxisLabel(ImAxis_X1, x_label);
        SetupAxisLabel(ImAxis_Y1, y_label);
    }
}