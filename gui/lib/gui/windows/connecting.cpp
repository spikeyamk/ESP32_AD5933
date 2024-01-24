#include <thread>
#include <chrono>
#include <cassert>
#include <cstring>
#include <list>
#include <iterator>
#include <vector>
#include <cstddef>
#include <stop_token>
#include <type_traits>

#include <trielo/trielo.hpp>

#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"
#include "gui/spinner.hpp"
#include "ad5933/masks/maps.hpp"
#include "magic/packets.hpp"

#include "gui/windows/connecting.hpp"

// For the console example, we are using a more C++ like approach of declaring a class to hold both data and functions.
struct ExampleAppConsole {
    char                  InputBuf[256];
    ImVector<char*>       Items;
    ImVector<const char*> Commands;
    ImVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;

    ExampleAppConsole()
    {
        ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;

        // "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
        Commands.push_back("HELP");
        Commands.push_back("HISTORY");
        Commands.push_back("CLEAR");
        Commands.push_back("CLASSIFY");
        AutoScroll = true;
        ScrollToBottom = false;
        AddLog("Welcome to Dear ImGui!");
    }
    ~ExampleAppConsole()
    {
        ClearLog();
        for (int i = 0; i < History.Size; i++)
            free(History[i]);
    }

    // Portable helpers
    static int   Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
    static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
    static char* Strdup(const char* s)                           { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
    static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

    void    ClearLog()
    {
        for (int i = 0; i < Items.Size; i++)
            free(Items[i]);
        Items.clear();
    }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        buf[IM_ARRAYSIZE(buf)-1] = 0;
        va_end(args);
        Items.push_back(Strdup(buf));
    }

    void    Draw(const char* title, bool* p_open)
    {
        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
        // So e.g. IsItemHovered() will return true when hovering the title bar.
        // Here we create a context menu only available from the title bar.
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Close Console"))
                *p_open = false;
            ImGui::EndPopup();
        }

        ImGui::TextWrapped(
            "This example implements a console with basic coloring, completion (TAB key) and history (Up/Down keys). A more elaborate "
            "implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
        ImGui::TextWrapped("Enter 'HELP' for help.");

        // TODO: display items starting from the bottom

        if (ImGui::SmallButton("Add Debug Text"))  { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); }
        ImGui::SameLine();
        if (ImGui::SmallButton("Add Debug Error")) { AddLog("[error] something went wrong"); }
        ImGui::SameLine();
        if (ImGui::SmallButton("Clear"))           { ClearLog(); }
        ImGui::SameLine();
        bool copy_to_clipboard = ImGui::SmallButton("Copy");
        //static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

        ImGui::Separator();

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }

        // Options, Filter
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
        ImGui::Separator();

        // Reserve enough left-over height for 1 separator + 1 input text
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::Selectable("Clear")) ClearLog();
                ImGui::EndPopup();
            }

            // Display every line as a separate entry so we can change their color or add custom widgets.
            // If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
            // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
            // to only process visible items. The clipper will automatically measure the height of your first item and then
            // "seek" to display only items in the visible area.
            // To use the clipper we can replace your standard loop:
            //      for (int i = 0; i < Items.Size; i++)
            //   With:
            //      ImGuiListClipper clipper;
            //      clipper.Begin(Items.Size);
            //      while (clipper.Step())
            //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            // - That your items are evenly spaced (same height)
            // - That you have cheap random access to your elements (you can access them given their index,
            //   without processing all the ones before)
            // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
            // We would need random-access on the post-filtered list.
            // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
            // or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
            // and appending newly elements as they are inserted. This is left as a task to the user until we can manage
            // to improve this example code!
            // If your items are of variable height:
            // - Split them into same height items would be simpler and facilitate random-seeking into your list.
            // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
            if (copy_to_clipboard)
                ImGui::LogToClipboard();
            for (const char* item : Items)
            {
                if (!Filter.PassFilter(item))
                    continue;

                // Normally you would store more information in your item than just a string.
                // (e.g. make Items[] an array of structure, store color/type etc.)
                ImVec4 color;
                bool has_color = false;
                if (strstr(item, "[error]")) { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
                else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
                if (has_color)
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::TextUnformatted(item);
                if (has_color)
                    ImGui::PopStyleColor();
            }
            if (copy_to_clipboard)
                ImGui::LogFinish();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                ImGui::SetScrollHereY(1.0f);
            ScrollToBottom = false;

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
        if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
        {
            char* s = InputBuf;
            Strtrim(s);
            if (s[0])
                ExecCommand(s);
            strcpy(s, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

        ImGui::End();
    }

    void    ExecCommand(const char* command_line)
    {
        AddLog("# %s\n", command_line);

        // Insert into history. First find match and delete it so it can be pushed to the back.
        // This isn't trying to be smart or optimal.
        HistoryPos = -1;
        for (int i = History.Size - 1; i >= 0; i--)
            if (Stricmp(History[i], command_line) == 0)
            {
                free(History[i]);
                History.erase(History.begin() + i);
                break;
            }
        History.push_back(Strdup(command_line));

        // Process command
        if (Stricmp(command_line, "CLEAR") == 0)
        {
            ClearLog();
        }
        else if (Stricmp(command_line, "HELP") == 0)
        {
            AddLog("Commands:");
            for (int i = 0; i < Commands.Size; i++)
                AddLog("- %s", Commands[i]);
        }
        else if (Stricmp(command_line, "HISTORY") == 0)
        {
            int first = History.Size - 10;
            for (int i = first > 0 ? first : 0; i < History.Size; i++)
                AddLog("%3d: %s\n", i, History[i]);
        }
        else
        {
            AddLog("Unknown command: '%s'\n", command_line);
        }

        // On command input, we scroll to bottom even if AutoScroll==false
        ScrollToBottom = true;
    }

    // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
    {
        ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
        return console->TextEditCallback(data);
    }

    int     TextEditCallback(ImGuiInputTextCallbackData* data)
    {
        //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
        case ImGuiInputTextFlags_CallbackCompletion:
            {
                // Example of TEXT COMPLETION

                // Locate beginning of current word
                const char* word_end = data->Buf + data->CursorPos;
                const char* word_start = word_end;
                while (word_start > data->Buf)
                {
                    const char c = word_start[-1];
                    if (c == ' ' || c == '\t' || c == ',' || c == ';')
                        break;
                    word_start--;
                }

                // Build a list of candidates
                ImVector<const char*> candidates;
                for (int i = 0; i < Commands.Size; i++)
                    if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
                        candidates.push_back(Commands[i]);

                if (candidates.Size == 0)
                {
                    // No match
                    AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
                }
                else if (candidates.Size == 1)
                {
                    // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0]);
                    data->InsertChars(data->CursorPos, " ");
                }
                else
                {
                    // Multiple matches. Complete as much as we can..
                    // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                    int match_len = (int)(word_end - word_start);
                    for (;;)
                    {
                        int c = 0;
                        bool all_candidates_matches = true;
                        for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                            if (i == 0)
                                c = toupper(candidates[i][match_len]);
                            else if (c == 0 || c != toupper(candidates[i][match_len]))
                                all_candidates_matches = false;
                        if (!all_candidates_matches)
                            break;
                        match_len++;
                    }

                    if (match_len > 0)
                    {
                        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                        data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                    }

                    // List matches
                    AddLog("Possible matches:\n");
                    for (int i = 0; i < candidates.Size; i++)
                        AddLog("- %s\n", candidates[i]);
                }

                break;
            }
        case ImGuiInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const int prev_history_pos = HistoryPos;
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (HistoryPos == -1)
                        HistoryPos = History.Size - 1;
                    else if (HistoryPos > 0)
                        HistoryPos--;
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (HistoryPos != -1)
                        if (++HistoryPos >= History.Size)
                            HistoryPos = -1;
                }

                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prev_history_pos != HistoryPos)
                {
                    const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history_str);
                }
            }
        }
        return 0;
    }
};

namespace GUI {
    namespace Windows {
        ImGuiID top_with_dock_space(MenuBarEnables &menu_bar_enables) {
            // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
            // because it would be confusing to have two docking targets within each others.
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("TopWindow", nullptr, window_flags);
            ImGui::PopStyleVar();
            ImGui::PopStyleVar(2);

            // DockSpace
            ImGuiIO& io = ImGui::GetIO();
            ImGuiID dockspace_id = ImGui::GetID("TopDockSpace");
            if(io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
                if(ImGui::BeginMenuBar()) {
                    if(ImGui::BeginMenu("View")) {
                        ImGui::MenuItem("BLE Client", nullptr, &menu_bar_enables.ble_client);
                        ImGui::MenuItem("Configure", nullptr, &menu_bar_enables.configure);
                        ImGui::MenuItem("Debug Registers", nullptr, &menu_bar_enables.debug_registers);
                        ImGui::MenuItem("Measurement Plots", nullptr, &menu_bar_enables.measurement_plots);
                        ImGui::MenuItem("Calibration Plots", nullptr, &menu_bar_enables.calibration_plots);
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }
                ImGui::End();
            }

            return dockspace_id;
        }

        void ble_client(bool &enable, ImGuiID left_id, int &selected, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
            if(enable == false) {
                return;
            }

            static bool first = true;
            if(first) {
                ImGui::DockBuilderDockWindow("BLE Client", left_id);
                first = false;
            }

            if(ImGui::Begin("BLE Client", &enable, ImGuiWindowFlags_NoMove) == false) {
                ImGui::End();
                return;
            }

            const auto show_table = [&selected, &shm]() {
                if(ImGui::BeginTable("Scan", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Identifier");
                    ImGui::TableNextColumn();
                    ImGui::Text("Address");
                    ImGui::TableNextColumn();
                    ImGui::Text("Status");
                    std::for_each(shm->discovery_devices->begin(), shm->discovery_devices->end(), [index = 0, &selected](const BLE_Client::Discovery::Device& e) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::PushID(index);
                        if(ImGui::Selectable(e.identifier, index == selected, ImGuiSelectableFlags_SpanAllColumns)) {
                            if(selected == index) {
                                selected = -1;
                            } else {
                                selected = index;
                            }
                        }
                        ImGui::PopID();
                        ImGui::TableNextColumn();
                        ImGui::Text(e.address);
                        ImGui::TableNextColumn();
                        if(e.connected) {
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
                        } else {
                            ImGui::Text("Disconnected");
                        }
                    });
                }
                ImGui::EndTable();
            };

            std::visit([&](auto&& active_state) {
                using T_Decay = std::decay_t<decltype(active_state)>;
                if constexpr (std::is_same_v<T_Decay, BLE_Client::Discovery::States::off>) {
                    if(ImGui::Button("Find adapter")) {
                        shm->cmd_deque->push_back(BLE_Client::Discovery::Events::find_default_active_adapter{});
                    }
                } else if constexpr (std::is_same_v<T_Decay, BLE_Client::Discovery::States::using_adapter>) {
                    if(ImGui::Button("Scan")) {
                        shm->cmd_deque->push_back(BLE_Client::Discovery::Events::start_discovery{});
                    }
                } else if constexpr (std::is_same_v<T_Decay, BLE_Client::Discovery::States::discovering>) {
                    if(ImGui::Button("Stop Scan")) {
                        shm->cmd_deque->push_back(BLE_Client::Discovery::Events::stop_discovery{});
                    }
                    ImGui::SameLine();
                    Spinner::Spinner("Scanning", 5.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                }
                if constexpr (!(std::is_same_v<T_Decay, BLE_Client::Discovery::States::off>
                || std::is_same_v<T_Decay, BLE_Client::Discovery::States::using_adapter>)) {
                    show_table();
                }

                if constexpr (std::is_same_v<T_Decay, BLE_Client::Discovery::States::discovered>) {
                    if(selected > -1) {
                        if(ImGui::Button("Connect")) {
                            shm->cmd_deque->push_back(BLE_Client::Discovery::Events::connect{ static_cast<size_t>(selected) });
                            std::thread([&]() {
                                bool its_connected = false;
                                for(int i = 0; i < 100 && its_connected == false; i++) { 
                                    shm->cmd_deque->push_back(BLE_Client::Discovery::Events::is_connected{});
                                    std::visit([&](auto&& extra_active_state) {
                                        using T_ExtraDecay = std::decay_t<decltype(extra_active_state)>;
                                        if constexpr (std::is_same_v<T_ExtraDecay, BLE_Client::Discovery::States::connected>) {
                                            its_connected = true;
                                            shm->cmd_deque->push_back(BLE_Client::Discovery::Events::is_esp32_ad5933{});
                                        }
                                    }, *shm->active_state);
                                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                }
                            }).detach();
                        }
                    }
                }
                if constexpr (std::is_same_v<T_Decay, BLE_Client::Discovery::States::connecting>) {
                    Spinner::Spinner("ClientSpinner", 5.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                }
            }, *shm->active_state);

            /*
            const auto to_lower = [](const std::string &string) { 
                auto ret = string;
                std::transform(ret.begin(), ret.end(), ret.begin(), [](unsigned char c) { return std::tolower(c); });
                return ret;
            };

            static char filter_content[32] = { '\0' };
            ImGui::InputText("##Filter", filter_content, sizeof(filter_content));
            if(std::strlen(filter_content) > 0) {
                std::vector<SimpleBLE::Peripheral> filtered_peripherals;
                filtered_peripherals.reserve(peripherals.size());
                std::string filter_string { to_lower(filter_content) };
                std::copy_if(peripherals.begin(), peripherals.end(), std::back_inserter(filtered_peripherals), [&to_lower, &filter_string](SimpleBLE::Peripheral e) {
                    if(to_lower(e.address().substr(0, filter_string.size())) == filter_string) {
                        return true;
                    } else if(to_lower(e.identifier().substr(0, filter_string.size())) == filter_string) {
                        return true;
                    } else {
                        return false;
                    }
                });
                show_connect_button(filtered_peripherals);
                show_table(filtered_peripherals);
            } else {
                show_connect_button(peripherals);
                show_table(peripherals);
            }
            */

            ImGui::End();
        }

        void console(int i, ImGuiConsole &console, ImGuiID side_id) {
            static bool drawn = false;
            if(drawn == false) {
                std::thread([&console]() {
                    while(1) {
                        console.System().Log(csys::ItemType::LOG_TYPE_INFO) << "Welcome to the imgui-console example!" << csys::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                }).detach();;
                drawn = true;
            }
            console.Draw();
        }

        bool send_packet(std::shared_ptr<ESP32_AD5933> esp32_ad5933, const Magic::Packets::Packet_T &packet) {
            const std::string footer_string(packet.begin(), packet.end());
            if(esp32_ad5933->send(footer_string) == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: send_simple_packet failed: {}\n", footer_string);
                return false;
            }
            return true;
        }

        template<size_t N>
        bool send_packet_and_footer(std::shared_ptr<ESP32_AD5933> esp32_ad5933, const std::array<uint8_t, N> &raw_array, const Magic::Packets::Packet_T &footer) {
            static_assert(N < Magic::Packets::Debug::start.size());
            Magic::Packets::Packet_T buf = footer;
            std::copy(raw_array.begin(), raw_array.end(), buf.begin());
            return send_packet(esp32_ad5933, buf);
        }

        void send_configure(Client &client) {
            if(send_packet_and_footer(client.esp32_ad5933, client.configure_captures.config.to_raw_array(), Magic::Packets::FrequencySweep::configure) == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: configure failed\n");
                client.configured = false;
            }
            client.configured = true;
        }

        void calibrate(std::stop_token st, Client &client) {
            std::unique_lock lock(client.esp32_ad5933->rx_payload.mutex);
            const uint16_t wished_size = client.configure_captures.config.get_num_of_inc().unwrap() + 1;
            const float progress_bar_step = 1.0f / static_cast<float>(wished_size);
            const float timeout_ms = (1.0f / static_cast<float>(client.configure_captures.config.get_start_freq().unwrap())) 
                * static_cast<float>(client.configure_captures.config.get_settling_time_cycles_number().unwrap())
                * AD5933::Masks::Or::SettlingTimeCyclesHB::get_multiplier_float(client.configure_captures.config.get_settling_time_cycles_multiplier())
                * 1'000'000.0f
                * 10.0f;
            const auto chrono_timeout_ms { std::chrono::milliseconds(static_cast<size_t>(timeout_ms)) };

            std::vector<AD5933::Data> tmp_raw_calibration;
            tmp_raw_calibration.reserve(wished_size);
            std::vector<AD5933::Calibration<float>> tmp_calibration;
            tmp_calibration.reserve(wished_size);

            client.progress_bar_fraction = 0.0f;

            if(send_packet(client.esp32_ad5933, Magic::Packets::FrequencySweep::run) == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: calibrate: send_packet failed\n");
                return;
            }

            do {
                client.esp32_ad5933->rx_payload.cv.wait_for(lock, chrono_timeout_ms, [&]() { 
                    if(client.esp32_ad5933->rx_payload.content.empty() == true) {
                        return false;
                    }

                    if(client.esp32_ad5933->rx_payload.content.front().size() < 4) {
                        return false;
                    }

                    std::array<uint8_t, 20> tmp_raw { 0x00 };
                    std::copy(client.esp32_ad5933->rx_payload.content.front().begin(), client.esp32_ad5933->rx_payload.content.front().end(), tmp_raw.begin());
                    if(Magic::Packets::get_magic_packet_pointer(tmp_raw) != Magic::Packets::FrequencySweep::read_data_valid_value) {
                        return false;
                    }

                    return true;
                });

                if(st.stop_requested()) {
                    client.calibrating = false;
    		        fmt::print(fmt::fg(fmt::color::yellow), "INFO: ESP32_AD5933: stopping calibration\n");
                    return;
                }

                if(client.esp32_ad5933->rx_payload.content.empty() == true) {
    		        fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: calibrate: failed: timeout\n");
                    return;
                }

                const auto rx_payload = client.esp32_ad5933->rx_payload.content.front();
                client.esp32_ad5933->rx_payload.content.pop();

                std::array<uint8_t, 4> raw_array;
                std::copy(rx_payload.begin(), rx_payload.begin() + 4, raw_array.begin());
                AD5933::Data tmp_data { raw_array };
                tmp_raw_calibration.push_back(tmp_data);
                AD5933::Calibration<float> tmp_cal { tmp_data, client.configure_captures.calibration_impedance };
                tmp_calibration.push_back(tmp_cal);
                client.progress_bar_fraction += progress_bar_step;
            } while(tmp_calibration.size() != wished_size);
            client.raw_calibration = tmp_raw_calibration;
            client.calibration = tmp_calibration;
            client.calibrated = true;
            client.calibrating = false;
        }

        void sweep(std::stop_token st, Client &client) {
            std::unique_lock lock(client.esp32_ad5933->rx_payload.mutex);
            const uint16_t wished_size = client.configure_captures.config.get_num_of_inc().unwrap() + 1;
            assert(wished_size <= client.calibration.size());
            const float progress_bar_step = 1.0f / static_cast<float>(wished_size);
            const float timeout_ms = (1.0f / static_cast<float>(client.configure_captures.config.get_start_freq().unwrap())) 
                * static_cast<float>(client.configure_captures.config.get_settling_time_cycles_number().unwrap())
                * AD5933::Masks::Or::SettlingTimeCyclesHB::get_multiplier_float(client.configure_captures.config.get_settling_time_cycles_multiplier())
                * 1'000'000.0f
                * 10.0f;
            const auto chrono_timeout_ms { std::chrono::milliseconds(static_cast<size_t>(timeout_ms)) };
            const std::vector<AD5933::Calibration<float>> tmp_calibration { client.calibration };
            do {
                std::vector<AD5933::Data> tmp_raw_measurement;
                tmp_raw_measurement.reserve(wished_size);
                std::vector<AD5933::Measurement<float>> tmp_measurement;
                tmp_measurement.reserve(wished_size);

                client.progress_bar_fraction = 0.0f;

                if(send_packet(client.esp32_ad5933, Magic::Packets::FrequencySweep::run) == false) {
                    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: sweep: send_packet: run: failed\n");
                    return;
                }
                do {
                    client.esp32_ad5933->rx_payload.cv.wait_for(lock, chrono_timeout_ms, [&]() { 
                        if(client.esp32_ad5933->rx_payload.content.empty() == true) {
                            return false;
                        }

                        if(client.esp32_ad5933->rx_payload.content.front().size() < 4) {
                            return false;
                        }

                        std::array<uint8_t, 20> tmp_raw { 0x00 };
                        std::copy(client.esp32_ad5933->rx_payload.content.front().begin(), client.esp32_ad5933->rx_payload.content.front().end(), tmp_raw.begin());
                        if(Magic::Packets::get_magic_packet_pointer(tmp_raw) != Magic::Packets::FrequencySweep::read_data_valid_value) {
                            return false;
                        }

                        return true;
                    });

                    if(st.stop_requested()) {
                        client.sweeping = false;
                        fmt::print(fmt::fg(fmt::color::yellow), "INFO: ESP32_AD5933: stopping sweep\n");
                        return;
                    }

                    if(client.esp32_ad5933->rx_payload.content.empty() == true) {
                        fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: sweep: failed: timeout\n");
                        return;
                    }

                    const auto rx_payload = client.esp32_ad5933->rx_payload.content.front();
                    client.esp32_ad5933->rx_payload.content.pop();

                    std::array<uint8_t, 4> raw_array;
                    std::copy(rx_payload.begin(), rx_payload.begin() + 4, raw_array.begin());
                    AD5933::Data tmp_data { raw_array };
                    tmp_raw_measurement.push_back(tmp_data);
                    AD5933::Measurement<float> tmp_meas { tmp_data, tmp_calibration[tmp_measurement.size()] };
                    tmp_measurement.push_back(tmp_meas);
                    client.progress_bar_fraction += progress_bar_step;
                } while(tmp_measurement.size() != wished_size);
                client.raw_measurement = tmp_raw_measurement;
                client.measurement = tmp_measurement;
                client.sweeped = true;
            } while(client.periodically_sweeping);
            client.sweeping = false;
        }

        void configure(int i, ImGuiID side_id, bool &enable, Client &client) {
            static char base[] = "Configure";
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

            const ImGuiInputTextFlags input_flags = (client.calibrating == true || client.sweeping == true) ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None;

            ImGui::Separator(); 
            ImGui::Text("Sweep Parameters");

            ImGui::Separator();

            ImGui::InputText(
                "Start Frequency",
                client.configure_captures.freq_start.data(),
                client.configure_captures.freq_start.size(),
                input_flags
            );
            ImGui::InputText(
                "Increment Frequency",
                client.configure_captures.freq_inc.data(),
                client.configure_captures.freq_inc.size(),
                input_flags
            );
            ImGui::InputText(
                "Number of Increments",
                client.configure_captures.num_of_inc.data(),
                client.configure_captures.num_of_inc.size(),
                input_flags
            );

            ImGui::Separator();

            ImGui::InputFloat(
                "Calibration impedance",
                &client.configure_captures.calibration_impedance,
                0.0f,
                0.0f,
                "%f",
                input_flags
            );

            ImGui::Separator(); 

            ImGui::Combo(
                "Output Excitation Voltage Range",
                &client.configure_captures.voltage_range_combo,
                "2 Vppk\0""1 Vppk\0""400 mVppk\0""200 mVppk\0"
            );

            ImGui::Separator();

            ImGui::Combo("PGA Gain", &client.configure_captures.pga_gain_combo, "x1\0""x5\0");

            ImGui::Separator();

            ImGui::InputText(
                "Number of Settling Time Cycles",
                client.configure_captures.settling_time_cycles_num.data(),
                client.configure_captures.settling_time_cycles_num.size(),
                input_flags
            );
            ImGui::Combo(
                "Settling Time Cycles Multiplier",
                &client.configure_captures.settling_time_cycles_multiplier_combo,
                "x1\0""x2\0""x4\0"
            );

            ImGui::Separator();

            ImGui::Combo("System Clock Source", &client.configure_captures.sysclk_src_combo, "Internal\0""External\0");
            ImGui::InputText(
                "System Clock Frequency",
                client.configure_captures.sysclk_freq.data(),
                client.configure_captures.sysclk_freq.size(),
                ImGuiInputTextFlags_ReadOnly
            );

            ImGui::Separator(); 

            if(client.debug_started == true) {
                client.configure_captures.update_config();
                ImGui::End();
                return;
            }

            if(ImGui::Button("Configure")) {
                std::jthread t1(send_configure, std::ref(client));
                t1.detach();
            }

            if(client.configured == true) {
                ImGui::SameLine();
                if(ImGui::Button("Freq Sweep End")) {
                    std::jthread t1([&]() { send_packet(client.esp32_ad5933, Magic::Packets::FrequencySweep::end); client.configured = false; });
                    t1.detach();
                } 

                if(client.calibrating == false && client.sweeping == false) {
                    if(ImGui::Button("Calibrate")) {
                        client.calibrating = true;
                        client.calibrated = false;
                        std::jthread t1(calibrate, client.ss.get_token(), std::ref(client));
                        t1.detach();
                    }
                } else if(client.sweeping == false) {
                    Spinner::Spinner("Calibrating", 10.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    ImGui::ProgressBar(client.progress_bar_fraction);
                }

                if(client.calibrated == true) {
                    ImGui::SameLine();
                    ImGui::Checkbox("Periodic Sweep", &client.periodically_sweeping);
                    if(client.sweeping == false) {
                        if(ImGui::Button("Start Sweep")) {
                            client.sweeping = true;
                            client.sweeped = false;
                            std::jthread t1(sweep, client.ss.get_token(), std::ref(client));
                            t1.detach();
                        }
                    } else if(client.calibrating == false) {
                        Spinner::Spinner("Measuring", 10.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                        ImGui::ProgressBar(client.progress_bar_fraction);
                    }
                }
            }

            client.configure_captures.update_config();
            ImGui::End();
        }

        bool dump(Client &client) {
            std::unique_lock lock(client.esp32_ad5933->rx_payload.mutex);
            if(send_packet(client.esp32_ad5933, Magic::Packets::Debug::dump_all_registers) == false) {
                return false;
            }
            if(client.esp32_ad5933->rx_payload.cv.wait_for(lock, std::chrono::milliseconds(1000), [&client, packet_footer = Magic::Packets::Debug::dump_all_registers]() {
                if(client.esp32_ad5933->rx_payload.content.empty()) {
                    return false;
                }
                if(client.esp32_ad5933->rx_payload.content.front().size() < 19) {
                    return false;
                }
                std::array<uint8_t, 20> tmp_raw { 0x00 };
                std::copy(client.esp32_ad5933->rx_payload.content.front().begin(), client.esp32_ad5933->rx_payload.content.front().end(), tmp_raw.begin());
                if(Magic::Packets::get_magic_packet_pointer(std::move(tmp_raw)) != packet_footer) {
                    return false;
                }
                return true;
            }) == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: Debug: Dump failed\n");
                return false;
            }
            client.debug_captures.update_captures(std::move(client.esp32_ad5933->rx_payload.content.front()));
            client.esp32_ad5933->rx_payload.content.pop();
            return true;
        }

        bool debug_program(Client &client) {
            if(send_packet_and_footer(client.esp32_ad5933, client.debug_captures.config.to_raw_array(), Magic::Packets::Debug::program_all_registers) == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: debug_program: failed\n");
                return false;
            }
            return true;
        }
        
        void program_and_dump(Client &client) {
            if(debug_program(client) == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: program_and_dump: program: failed\n");
                return;
            }
            if(dump(client) == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: program_and_dump: dump: failed\n");
            }
        }

        void command_and_dump(Client &client, AD5933::Masks::Or::Ctrl::HB::Command command) {
            auto buf = Magic::Packets::Debug::control_HB_command;
            buf[0] = static_cast<uint8_t>(command);
            if(send_packet(client.esp32_ad5933, buf) == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: command_and_dump: send_packet: {} failed\n", static_cast<uint8_t>(command));
            }
            if(dump(client) == false) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ESP32_AD5933: command_and_dump: dump: {} failed\n", static_cast<uint8_t>(command));
            }
        }

        void debug_registers(int i, ImGuiID side_id, bool &enable, Client &client) {
            std::string name = "Debug Registers##" + std::to_string(i);
            static int first = 0;
            if(first == i) {
                ImGui::DockBuilderDockWindow(name.c_str(), side_id);
                first++;
            }

            if(ImGui::Begin(name.c_str(), &enable) == false) {
                ImGui::End();
                return;
            }

            if(client.configured == false) {
                if(client.debug_started == false) {
                    if(ImGui::Button("Start")) {
                        std::jthread t1([&]() { if(send_packet(client.esp32_ad5933, Magic::Packets::Debug::start) == true) { client.debug_started = true; }} );
                        t1.detach();
                    }
                } else {
                    if(ImGui::Button("Dump")) {
                        std::jthread t1(dump, std::ref(client));
                        t1.detach();
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Program")) {
                        std::jthread t1(program_and_dump, std::ref(client));
                        t1.detach();
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("End")) {
                        std::jthread t1([&]() { send_packet(client.esp32_ad5933, Magic::Packets::Debug::end); client.debug_started = false; });
                        t1.detach();
                    }
                }
            }

            ImGui::Separator();

            ImGui::Text("ControlHB:");
            ImGui::Text("\tControlHB Command: %s", AD5933::Masks::get_map_str(client.debug_captures.config.get_command(), AD5933::Masks::Or::Ctrl::HB::command_map));
            ImGui::Text("\tExcitation Output Voltage Range: %s", AD5933::Masks::get_map_str(client.debug_captures.config.get_voltage_range(), AD5933::Masks::Or::Ctrl::HB::voltage_map));
            ImGui::Text("\tPGA Gain: %s", AD5933::Masks::get_map_str(client.debug_captures.config.get_PGA_gain(), AD5933::Masks::Or::Ctrl::HB::pga_gain_map));
            ImGui::Separator();

            ImGui::Text("ControlLB:");
            ImGui::Text("\tSystem Clock Source: %s", AD5933::Masks::get_map_str(client.debug_captures.config.get_sysclk_src(), AD5933::Masks::Or::Ctrl::LB::sysclk_src_map)); 
            ImGui::Separator();

            ImGui::Text("Start Frequency: %f", client.debug_captures.config.get_start_freq().to_float());
            ImGui::Text("Frequency Increment: %f", client.debug_captures.config.get_inc_freq().to_float());
            ImGui::Text("Number of Increments: %u", client.debug_captures.config.get_num_of_inc().unwrap());
            ImGui::Text("Number of Settling Time Cycles: %u", client.debug_captures.config.get_settling_time_cycles_number().unwrap());
            ImGui::Text("Settling Time Cycles Multiplier: %s", AD5933::Masks::get_map_str(client.debug_captures.config.get_settling_time_cycles_multiplier(), AD5933::Masks::Or::SettlingTimeCyclesHB::multiplier_map));
            ImGui::Separator();
            ImGui::Text("Status: %s", AD5933::Masks::get_map_str(client.debug_captures.data.get_status(), AD5933::Masks::Or::status_map));
            ImGui::Separator();

            ImGui::InputText("Temperature [°C]",
                std::to_string(client.debug_captures.data.get_temperature()).data(),
                std::to_string(client.debug_captures.data.get_temperature()).size(),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::InputText("Real Part [1/Ohm]",
                std::to_string(client.debug_captures.data.get_real_part()).data(),
                std::to_string(client.debug_captures.data.get_real_part()).size(),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::InputText("Imaginary Part [1/Ohm]",
                std::to_string(client.debug_captures.data.get_imag_part()).data(),
                std::to_string(client.debug_captures.data.get_imag_part()).size(),
                ImGuiInputTextFlags_ReadOnly
            );
            ImGui::InputText("Raw Magnitude [1/Ohm]",
                std::to_string(client.debug_captures.data.get_raw_magnitude()).data(),
                std::to_string(client.debug_captures.data.get_raw_magnitude()).size(),
                ImGuiInputTextFlags_ReadOnly
            );
            static const auto float_to_str_lambda = [](double number) -> std::string {
                std::ostringstream stream;
                // Calculate the magnitude of the number
                double magnitude = std::abs(number);
                int precision = (magnitude == 0.0f) ? 0 : -static_cast<int>(std::floor(std::log10(magnitude))) + 7;
                stream << std::setprecision(precision) << number;
                return stream.str();
            };
            ImGui::InputText("Raw Phase [rad]",
                float_to_str_lambda(client.debug_captures.data.get_raw_phase()).data(),
                float_to_str_lambda(client.debug_captures.data.get_raw_phase()).size(),
                ImGuiInputTextFlags_ReadOnly
            );

            ImGui::Separator();

            ImGui::Text("Read/Write Registers");
            {
                ImGui::Separator();
                ImGui::InputText("CTRL_HB (0x80)", client.debug_captures.ctrl_HB.data(), client.debug_captures.ctrl_HB.size());
                ImGui::InputText("CTRL_LB (0x81)", client.debug_captures.ctrl_LB.data(), client.debug_captures.ctrl_LB.size());
                ImGui::Separator();
                ImGui::InputText("FREQ_START_HB (0x82)", client.debug_captures.freq_start_HB.data(), client.debug_captures.freq_start_HB.size());
                ImGui::InputText("FREQ_START_MB (0x83)", client.debug_captures.freq_start_MB.data(), client.debug_captures.freq_start_MB.size());
                ImGui::InputText("FREQ_START_LB (0x84)", client.debug_captures.freq_start_LB.data(), client.debug_captures.freq_start_LB.size());
                ImGui::Separator();
                ImGui::InputText("FREQ_INC_HB (0x85)", client.debug_captures.freq_inc_HB.data(), client.debug_captures.freq_inc_HB.size());
                ImGui::InputText("FREQ_INC_MB (0x86)", client.debug_captures.freq_inc_MB.data(), client.debug_captures.freq_inc_MB.size());
                ImGui::InputText("FREQ_INC_LB (0x87)", client.debug_captures.freq_inc_LB.data(), client.debug_captures.freq_inc_LB.size());
                ImGui::Separator();
                ImGui::InputText("NUM_OF_INC_HB (0x88)", client.debug_captures.num_of_inc_HB.data(), client.debug_captures.num_of_inc_HB.size());
                ImGui::InputText("NUM_OF_INC_LB (0x89)", client.debug_captures.num_of_inc_LB.data(), client.debug_captures.num_of_inc_LB.size());
                ImGui::Separator();
                ImGui::InputText("NUM_OF_SETTLING_TIME_CYCLES_HB (0x8A)", client.debug_captures.settling_time_cycles_HB.data(), client.debug_captures.settling_time_cycles_HB.size());
                ImGui::InputText("NUM_OF_SETTLING_TIME_CYCLES_LB (0x8B)", client.debug_captures.settling_time_cycles_LB.data(), client.debug_captures.settling_time_cycles_LB.size());

                ImGui::Separator();

            {
                ImGui::Text("Send Control Register Command Controls");
                if(ImGui::Button("Power-down mode")) {
                    std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode);
                    t1.detach();
                } ImGui::SameLine();
                    if(ImGui::Button("Standby mode")) {
                        std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::StandbyMode);
                        t1.detach();
                    } ImGui::SameLine();
                    if(ImGui::Button("No operation (NOP_0)")) {
                        std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::Nop_0);
                        t1.detach();
                    }

                if(ImGui::Button("Measure temperature")) {
                    std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::MeasureTemp);
                    t1.detach();
                }

                if(ImGui::Button("Initialize with start frequency")) {
                    std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::InitStartFreq);
                    t1.detach();
                } ImGui::SameLine();
                    if(ImGui::Button("Start frequency sweep")) {
                        std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::StartFreqSweep);
                        t1.detach();
                    } ImGui::SameLine();
                    if(ImGui::Button("Increment frequency")) {
                        std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::IncFreq);
                        t1.detach();
                    } ImGui::SameLine();
                    if(ImGui::Button("Repeat frequency")) {
                        std::jthread t1(command_and_dump, std::ref(client), AD5933::Masks::Or::Ctrl::HB::Command::RepeatFreq);
                        t1.detach();
                    }
                }

                ImGui::Separator();

                ImGui::Text("Special Status Register");
                ImGui::InputText("STATUS (0x8F)", client.debug_captures.status_capture.data(), client.debug_captures.status_capture.size(), ImGuiInputTextFlags_ReadOnly);

                ImGui::Separator();

                ImGui::Text("Read-only Data Registers");
                ImGui::InputText("TEMP_DATA_HB (0x92)", client.debug_captures.temp_data_HB.data(), client.debug_captures.temp_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::InputText("TEMP_DATA_LB (0x93)", client.debug_captures.temp_data_LB.data(), client.debug_captures.temp_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::Separator();
                ImGui::InputText("REAL_DATA_HB (0x94)", client.debug_captures.real_data_HB.data(), client.debug_captures.real_data_HB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::InputText("REAL_DATA_LB (0x95)", client.debug_captures.real_data_LB.data(), client.debug_captures.real_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::Separator();
                ImGui::InputText("IMAG_DATA_HB (0x96)", client.debug_captures.imag_data_HB.data(), client.debug_captures.imag_data_HB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::InputText("IMAG_DATA_LB (0x97)", client.debug_captures.imag_data_LB.data(), client.debug_captures.imag_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::Separator();
            }

            client.debug_captures.update_config();

            ImGui::End();
        }

        void calibration_plots(int i, ImGuiID side_id, bool &enable, Client &client) {
            std::string name = "Calibration Plots##" + std::to_string(i);
            static int first = 0;
            if(first == i) {
                ImGui::DockBuilderDockWindow(name.c_str(), side_id);
            }

            ImPlot::CreateContext();
            if(ImGui::Begin(name.c_str(), &enable) == false) {
                ImGui::End();
                return;
            }

            if(first == i) {
                ImGui::End();
                first++;
                return;
            }

            const auto start_freq = client.configure_captures.config.get_start_freq();
            const auto inc_freq = client.configure_captures.config.get_inc_freq();
            std::vector<float> frequency_vector(client.calibration.size());
            std::generate(
                frequency_vector.begin(),
                frequency_vector.end(),
                [start_freq, inc_freq, n = 0.0f] () mutable {
                    return static_cast<float>(start_freq.unwrap() + ((n++) * inc_freq.unwrap()));
                }
            );

            if(ImGui::BeginTabBar("Single_Plots")) {
                if(ImGui::BeginTabItem("Single")) {
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabBar("Calibration_PlotsBar")) {
                    if(ImGui::BeginTabItem("RAW_DATA")) {
                        std::vector<float> real_data_vector;
                        real_data_vector.reserve(client.raw_calibration.size());
                        std::transform(client.raw_calibration.begin(), client.raw_calibration.end(), real_data_vector.begin(), [](AD5933::Data &e) { return static_cast<float>(e.get_real_data()); });
                        if(ImPlot::BeginPlot("Calibration Raw Real Data")) {
                            ImPlot::SetupAxes("f [Hz]", "REAL_DATA");
                            ImPlot::PlotLine("REAL_DATA [1/Ohm]", frequency_vector.data(), real_data_vector.data(), frequency_vector.size());
                            ImPlot::EndPlot();
                        }

                        std::vector<float> imag_data_vector;
                        imag_data_vector.reserve(client.raw_calibration.size());
                        std::transform(client.raw_calibration.begin(), client.raw_calibration.end(), imag_data_vector.begin(), [](AD5933::Data &e) { return static_cast<float>(e.get_imag_data()); });
                        if(ImPlot::BeginPlot("Calibration Raw Imag Data")) {
                            ImPlot::SetupAxes("f [Hz]", "IMAG_DATA");
                            ImPlot::PlotLine("IMAG_DATA [1/Ohm]", frequency_vector.data(), imag_data_vector.data(), frequency_vector.size());
                            ImPlot::EndPlot();
                        }

                        if(ImGui::BeginTable("RawData", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
                            ImGui::TableSetupColumn("Frequency [Hz]");
                            ImGui::TableSetupColumn("Real [1/Ohm]");
                            ImGui::TableSetupColumn("Imag [1/Ohm]");
                            ImGui::TableHeadersRow();
                            std::for_each(frequency_vector.begin(), frequency_vector.end(), [index = 0,  &real_data_vector, &imag_data_vector](const float &f) mutable {
                                ImGui::TableNextRow();
                                if(index == 0) {
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                    ImGui::TableSetColumnIndex(2);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                }

                                ImGui::TableNextColumn();
                                std::string frequency_string { std::to_string(f) };
                                ImGui::PushID(index);
                                ImGui::InputText("##Frequency", frequency_string.data(), frequency_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::TableNextColumn();
                                std::string real_string { std::to_string(real_data_vector[index]) };
                                ImGui::InputText("##RealData", real_string.data(), real_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::TableNextColumn();
                                std::string imag_string { std::to_string(imag_data_vector[index]) };
                                ImGui::InputText("##ImagData", imag_string.data(), imag_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::PopID();
                                index++;
                            });
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("CALCULATED_DATA")) {
                        std::vector<float> raw_magnitude_vector;
                        raw_magnitude_vector.reserve(client.raw_calibration.size());
                        std::transform(client.raw_calibration.begin(), client.raw_calibration.end(), raw_magnitude_vector.begin(), [](AD5933::Data &e) { return e.get_raw_magnitude<float>(); });
                        if(ImPlot::BeginPlot("Calibration Calculated Magnitude")) {
                            ImPlot::SetupAxes("f [Hz]", "RAW_MAGNITUDE");
                            ImPlot::PlotLine("RAW_MAGNITUDE [1/Ohm]", frequency_vector.data(), raw_magnitude_vector.data(), frequency_vector.size());
                            ImPlot::EndPlot();
                        }

                        std::vector<float> raw_phase_vector;
                        raw_phase_vector.reserve(client.raw_calibration.size());
                        std::transform(client.raw_calibration.begin(), client.raw_calibration.end(), raw_phase_vector.begin(), [](AD5933::Data &e) { return e.get_raw_phase<float>(); });
                        if(ImPlot::BeginPlot("Calibration Calculated Phase")) {
                            ImPlot::SetupAxes("f [Hz]", "RAW_PHASE");
                            ImPlot::PlotLine("RAW_PHASE [rad]", frequency_vector.data(), raw_phase_vector.data(), frequency_vector.size());
                            ImPlot::EndPlot();
                        }

                        if(ImGui::BeginTable("RawData", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
                            ImGui::TableSetupColumn("Frequency [Hz]");
                            ImGui::TableSetupColumn("Raw Magnitude [1/Ohm]");
                            ImGui::TableSetupColumn("Raw Phase [rad]");
                            ImGui::TableHeadersRow();
                            std::for_each(frequency_vector.begin(), frequency_vector.end(), [index = 0,  &raw_magnitude_vector, &raw_phase_vector](const float &f) mutable {
                                ImGui::TableNextRow();
                                if(index == 0) {
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                    ImGui::TableSetColumnIndex(2);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                }

                                ImGui::TableNextColumn();
                                std::string frequency_string { std::to_string(f) };
                                ImGui::PushID(index);
                                ImGui::InputText("##Frequency", frequency_string.data(), frequency_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::TableNextColumn();
                                std::string real_string { std::to_string(raw_magnitude_vector[index]) };
                                ImGui::InputText("##RealData", real_string.data(), real_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::TableNextColumn();
                                std::string imag_string { std::to_string(raw_phase_vector[index]) };
                                ImGui::InputText("##ImagData", imag_string.data(), imag_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::PopID();
                                index++;
                            });
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("CALIBRATION_DATA")) {
                        std::vector<float> gain_factor_vector;
                        gain_factor_vector.reserve(client.calibration.size());
                        std::transform(
                            client.calibration.begin(),
                            client.calibration.end(),
                            gain_factor_vector.begin(),
                            [](AD5933::Calibration<float> &e) {
                                return static_cast<float>(e.get_gain_factor());
                            }
                        );
                        if(ImPlot::BeginPlot("Calibration Gain Factor")) {
                            ImPlot::SetupAxes("f [Hz]", "GAIN_FACTOR");
                            ImPlot::PlotLine("GAIN_FACTOR", frequency_vector.data(), gain_factor_vector.data(), frequency_vector.size());
                            ImPlot::EndPlot();
                        }

                        std::vector<float> system_phase_vector;
                        system_phase_vector.reserve(client.calibration.size());
                        std::transform(
                            client.calibration.begin(),
                            client.calibration.end(),
                            system_phase_vector.begin(),
                            [](AD5933::Calibration<float> &e) {
                                return static_cast<float>(e.get_system_phase()); 
                            }
                        );
                        if(ImPlot::BeginPlot("Calibration System Phase")) {
                            ImPlot::SetupAxes("f [Hz]", "SYSTEM_PHASE");
                            ImPlot::PlotLine("SYSTEM_PHASE [rad]", frequency_vector.data(), system_phase_vector.data(), frequency_vector.size());
                            ImPlot::EndPlot();
                        }

                        if(ImGui::BeginTable("RawData", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
                            ImGui::TableSetupColumn("Frequency [Hz]");
                            ImGui::TableSetupColumn("Gain Factor");
                            ImGui::TableSetupColumn("System Phase [rad]");
                            ImGui::TableHeadersRow();
                            std::for_each(frequency_vector.begin(), frequency_vector.end(), [index = 0,  &gain_factor_vector, &system_phase_vector](const float &f) mutable {
                                ImGui::TableNextRow();
                                if(index == 0) {
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                    ImGui::TableSetColumnIndex(2);
                                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                                }

                                ImGui::TableNextColumn();
                                std::string frequency_string { std::to_string(f) };
                                ImGui::PushID(index);
                                ImGui::InputText("##Frequency", frequency_string.data(), frequency_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::TableNextColumn();
                                std::string gain_factor_string { std::to_string(gain_factor_vector[index]) };
                                ImGui::InputText("##GainFactor", gain_factor_string.data(), gain_factor_vector.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::TableNextColumn();
                                std::string system_phase_string { std::to_string(system_phase_vector[index]) };
                                ImGui::InputText("##SystemPhase", system_phase_string.data(), system_phase_string.size(), ImGuiInputTextFlags_ReadOnly);
                                ImGui::PopID();
                                index++;
                            });
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabBar();
            }

            ImGui::End();
        }

        void measurement_plots(int i, ImGuiID side_id, bool &enable) {
            static char base[] = "Measurement Plots";
            char name[30];
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
            ImGui::End();
        }

        DockspaceIDs split_left_center(ImGuiID dockspace_id) {
            const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace;
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);
            ImGuiID dock_id_center;
            ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dock_id_center);
            return { dock_id_left, dock_id_center };
        }

        void client1(int i, ImGuiID center_id, Client &client, MenuBarEnables &enables) {
            if(client.enable == false) {
                return;
            }

            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove;

            //window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            //window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
            // and handle the pass-thru hole, so we ask Begin() to not render a background.
            if(dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

            std::string name = client.esp32_ad5933->peripheral.identifier() + "##" + std::to_string(i);

            static int first = 0;
            if(first == i) {
                ImGui::DockBuilderDockWindow(name.c_str(), center_id);
            }

            ImGui::Begin(name.c_str(), &(client.enable), window_flags);

            if(client.enable == false) {
                ImGui::End();
                return;
            }

            // Submit the DockSpace
            ImGuiIO& io = ImGui::GetIO();
            if(io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
                std::string dockspace_name = "DockSpace" + client.esp32_ad5933->peripheral.identifier() + "##" + std::to_string(i);
                ImGuiID dockspace_id = ImGui::GetID(dockspace_name.c_str());
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
                if(first == i) {
                    static const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace;
                    ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
                    ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
                    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());
                    ImGuiID right_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);
                    measurement_plots(i, right_id, enables.measurement_plots);
                    calibration_plots(i, right_id, enables.calibration_plots, client);
                    configure(i, dockspace_id, enables.configure, client);
                    debug_registers(i, dockspace_id, enables.debug_registers, client);
                    first++;
                    ImGui::DockBuilderFinish(dockspace_id);
                } else {
                    if(enables.measurement_plots) {
                        measurement_plots(i, ImGui::GetID(static_cast<void*>(nullptr)), enables.measurement_plots);
                    }
                    if(enables.calibration_plots) {
                        calibration_plots(i, ImGui::GetID(static_cast<void*>(nullptr)), enables.calibration_plots, client);
                    }
                    if(enables.configure) {
                        configure(i, ImGui::GetID(static_cast<void*>(nullptr)), enables.configure, client);
                    }
                    if(enables.debug_registers) {
                        debug_registers(i, ImGui::GetID(static_cast<void*>(nullptr)), enables.debug_registers, client);
                    }
                }
            }

            ImGui::End();
        }

    }
}
