#pragma once

#include "gui/windows/client.hpp"

#include "imgui.h"

namespace GUI {
    namespace Windows {
        void measurement_plots(ImGuiID side_id, bool &enable, Client& client);
    }
}
