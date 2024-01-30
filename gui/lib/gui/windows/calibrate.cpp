#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include "gui/windows/calibrate.hpp"

namespace GUI {
    namespace Windows {
        void calibrate(int i, ImGuiID side_id, bool &enable, Client &client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
            static char base[] = "Calibrate";
            char name[20];
            std::sprintf(name, "%s##%d", base, i);

            static int first = 0;
            if(first == i) {
                ImGui::DockBuilderDockWindow(name, side_id);
                first++;
            }

            if(ImGui::Begin(name, &enable) == false) {
                ImGui::End();
                return;
            }

            ImGui::Text("Start Frequency (min 1'000 Hz)");
            ImGui::Text("End Frequency (inclusive) (max 100'000 Hz)");
            ImGui::Text("Increment Frequency (min 0.1 Hz)");
            ImGui::Text("Calibration Impedance (Ohm)");
            ImGui::Text("Output Excitation Voltage Range");
            ImGui::Text("PGA Gain");
            ImGui::Text("Settling time cycles create like a maybe a slider or something so that it will also calculate the time it will take to complete a measurement");
            ImGui::Text("Time for one measurement?");
            ImGui::Text("ETA?");
            ImGui::Text("System Clock Source");
            ImGui::Text("System Clock Frequency");

            ImGui::End();
        }
    }
}