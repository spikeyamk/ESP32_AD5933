#pragma once

#include "imgui.h"

#include "gui/windows/client.hpp"

namespace GUI {
    namespace Windows {
        void calibration_plots(int i, ImGuiID side_id, bool &enable, Client &client);
    }
}