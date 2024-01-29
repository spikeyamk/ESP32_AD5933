#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <boost/process.hpp>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <ble_client/standalone/shm.hpp>

namespace GUI {
    class Console {
    private:
        std::vector<std::string> lines { "first_line" };
        std::string text;
        bool auto_scroll = true;
        bool& enable;
        boost::process::ipstream& stdout_stream;
        boost::process::ipstream& stderr_stream;
        std::mutex mutex;
    public:
        inline Console(bool& enable, boost::process::ipstream& stdout_stream, boost::process::ipstream& stderr_stream) :
            enable { enable },
            stdout_stream{ stdout_stream },
            stderr_stream{ stderr_stream }
        {}

        inline void log(const std::string& text) {
            lines.push_back(text);
        }

        inline void draw() {
            if(enable == false) {
                return;
            }

            if(ImGui::Begin("Console", &enable) == false) {
                ImGui::End();
                return;
            }

            update_text();
            if(ImGui::InputTextMultiline("##source", &text, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput, nullptr, nullptr)) {
                std::printf("we here\n");
            }

            /* This snippet scrolls to the bottom of the InputTextMultiline https://github.com/ocornut/imgui/issues/5484#issuecomment-1189989347 */
            ImGuiContext& g = *GImGui;
            const char* child_window_name = NULL;
            ImFormatStringToTempBuffer(&child_window_name, NULL, "%s/%s_%08X", g.CurrentWindow->Name, "##source", ImGui::GetID("##source"));
            ImGuiWindow* child_window = ImGui::FindWindowByName(child_window_name);
            ImGui::SetScrollY(child_window, child_window->ScrollMax.y);

            ImGui::End();
        }

        inline void read_stdout() {
            /* https://www.boost.org/doc/libs/1_64_0/doc/html/process.html */
            std::string line;
            while(std::getline(stdout_stream, line) && !line.empty()) {
                std::cout << line << std::endl;
                //log(line);
            }
        }

        inline void read_stderr() {
            /* https://www.boost.org/doc/libs/1_64_0/doc/html/process.html */
            std::string line;
            while(std::getline(stderr_stream, line) && !line.empty()) {
                std::cout << line << std::endl;
                //log(line);
            }
        }
    private:
        inline void update_text() {
            for(const auto& e: lines) {
                if(e.empty() == false) {
                    text.append(e + "\n");
                }
            }
            lines.clear();
        }
    };
}
