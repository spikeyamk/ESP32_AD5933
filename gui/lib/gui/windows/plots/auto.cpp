#include "imgui_internal.h"
#include "implot.h"

#include "gui/boilerplate.hpp"

#include "gui/windows/plots/auto.hpp"

namespace GUI {
    namespace Windows {
        namespace Plots {
            Auto::Auto(const size_t index) :
                index { index }
            {
                name.append(std::to_string(index));
            }

            void Auto::draw(bool& enable, const ImGuiID side_id) {
                if(first) {
                    ImGui::DockBuilderDockWindow(name.c_str(), side_id);
                    ImPlot::CreateContext();
                    /*
                    const float scale = Boilerplate::get_scale();
                    ImPlot::GetStyle().PlotDefaultSize.x *= scale;
                    ImPlot::GetStyle().PlotDefaultSize.y *= scale;
                    */
                }

                if(ImGui::Begin(name.c_str(), &enable) == false) {
                    ImGui::End();
                    return;
                }

                if(first) {
                    ImGui::End();
                    first = false;
                    return;
                }

                if(ImGui::BeginTabBar("AUTO_PLOTS")) {
                    if(ImGui::BeginTabItem("SEND")) {
                        if(ImGui::BeginTabBar("SEND_TAB_BAR")) {
                            if(ImGui::BeginTabItem("CORRECTED_GON_DATA")) {
                                draw_corrected_gon_data();
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CORRECTED_ALG_DATA")) {
                                draw_corrected_alg_data();
                                ImGui::EndTabItem();
                            }
                            ImGui::EndTabBar();
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("SAVE")) {
                        if(ImGui::BeginTabBar("SAVE_TAB_BAR")) {
                            if(ImGui::BeginTabItem("CORRECTED_GON_DATA")) {
                                ImGui::EndTabItem();
                            }
                            if(ImGui::BeginTabItem("CORRECTED_ALG_DATA")) {
                                ImGui::EndTabItem();
                            }
                            ImGui::EndTabBar();
                        }
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }

                ImGui::End();
            }

            void Auto::draw_corrected_gon_data() {

            }

            void Auto::draw_corrected_alg_data() {

            }
        }
    }
}
