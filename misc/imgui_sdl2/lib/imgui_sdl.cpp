#include <iostream>
#include <sstream>
#include <chrono>
#include <functional>
#include <tuple>
#include <memory>
#include <optional>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <vector>
#include <numeric>

#include "trielo.hpp"
#include "fmt/core.h"
#include "fmt/color.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "implot.h"
#include "magic_packets.hpp"
#include "types.hpp"

#include "dark_mode_switcher.hpp"
#include "imgui_sdl.hpp"
#include "ble.hpp"
#include "ad5933_config.hpp"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

namespace ImGui {
    /* https://github.com/ocornut/imgui/issues/1901#issue-335266223 */
    bool BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;
        
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = size_arg;
        size.x -= style.FramePadding.x * 2;
        
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;
        
        // Render
        const float circleStart = size.x * 0.7f;
        const float circleEnd = size.x;
        const float circleWidth = circleEnd - circleStart;
        
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart*value, bb.Max.y), fg_col);
        
        const float t = static_cast<float>(g.Time);
        const float r = size.y / 2.0f;
        const float speed = 1.5f;
        
        const float a = speed * 0.0f;
        const float b = speed * 0.333f;
        const float c = speed * 0.666f;
        
        const float o1 = (circleWidth+r) * (t+a - speed * (int)((t+a) / speed)) / speed;
        const float o2 = (circleWidth+r) * (t+b - speed * (int)((t+b) / speed)) / speed;
        const float o3 = (circleWidth+r) * (t+c - speed * (int)((t+c) / speed)) / speed;
        
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);
        return true;
    }

    bool Spinner(const char* label, float radius, float thickness, const ImU32& color) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;
        
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size((radius )*2, (radius + style.FramePadding.y)*2);
        
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;
        
        // Render
        window->DrawList->PathClear();
        
        int num_segments = 30;
        float start = abs(ImSin(static_cast<float>(g.Time) * 1.8f) * (num_segments - 5));
        
        const float a_min = IM_PI * 2.0f * ((float) start) / (float) num_segments;
        const float a_max = IM_PI * 2.0f * ((float) num_segments - 3) / (float) num_segments;

        const ImVec2 centre = ImVec2(pos.x+radius, pos.y+radius+style.FramePadding.y);
        
        for (int i = 0; i < num_segments; i++) {
            const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
            window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + static_cast<float>(g.Time) * 8) * radius,
                                                centre.y + ImSin(a + static_cast<float>(g.Time) * 8) * radius));
        }

        window->DrawList->PathStroke(color, false, thickness);
        return true;
    }
}

namespace ImGuiSDL {
    // ImGuiSDL utility boilerplate functions
    static const auto sdl_error_lambda = []() { std::cout << SDL_GetError() << std::endl; };

    bool check_version() {
        return IMGUI_CHECKVERSION();
    }

    std::tuple<SDL_Window*, SDL_Renderer*> init() {
        Trielo::trieloxit_lambda<SDL_Init>(Trielo::OkErrCode(0), sdl_error_lambda, SDL_INIT_VIDEO | SDL_INIT_TIMER);
        Trielo::trielo_lambda<SDL_SetHint>(Trielo::OkErrCode(SDL_TRUE), sdl_error_lambda, SDL_HINT_IME_SHOW_UI, "1");

        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN);
        SDL_Window* window = Trielo::trieloxit_lambda<SDL_CreateWindow>(
            Trielo::FailErrCode(static_cast<SDL_Window*>(nullptr)),
            sdl_error_lambda,
            "AD5933",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            1280,
            720,
            window_flags
        );

        SDL_Renderer* renderer = Trielo::trieloxit_lambda<SDL_CreateRenderer>(
            Trielo::FailErrCode(static_cast<SDL_Renderer*>(nullptr)),
            sdl_error_lambda,
            window,
            -1,
            SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED
        );
        {
            SDL_RendererInfo info;
            Trielo::trielo_lambda<SDL_GetRendererInfo>(
                Trielo::OkErrCode(0),
                sdl_error_lambda,
                renderer,
                &info
            );
            SDL_Log("Current SDL_Renderer: %s", info.name);
        }

        Trielo::trieloxit<check_version>(Trielo::OkErrCode(true));
        if(ImGui::CreateContext() == nullptr) {
            std::cout << "ImGui failed to create context\n";
        }

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);

        /*
        DarkModeSwitcher dark_mode_switcher { window };
        if(DarkModeSwitcher::isGlobalDarkModeActive()) {
            ImGui::StyleColorsDark();
        } else {
            ImGui::StyleColorsLight();
        }
        */
        //Commented out because of a bug in a lambda capture of this and segfault on when darkModeRevoker is fired
        //dark_mode_switcher.enable_dynamic_switching();

        ImGui::StyleColorsDark();
        SDL_MaximizeWindow(window);
        return std::tuple { window, renderer };
    } 

    void shutdown(SDL_Renderer* renderer, SDL_Window* window) {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    inline void render(SDL_Renderer* renderer, const ImVec4& clear_color) {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Render();
        if(SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y) != 0) {
            fmt::print(fmt::fg(fmt::color::yellow), "WARNING: SDL_RenderSetScale failed");
            sdl_error_lambda();
        }
        if(SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255)) != 0) {
            fmt::print(fmt::fg(fmt::color::yellow), "WARNING: SDL_SetRenderDrawColor failed");
            sdl_error_lambda();
        }
        if(SDL_RenderClear(renderer) != 0) {
            fmt::print(fmt::fg(fmt::color::yellow), "WARNING: SDL_RenderClear failed\n");
            sdl_error_lambda();
        }
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }

    inline void process_events(SDL_Window* window, bool &done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if(event.type == SDL_QUIT) {
                done = true;
            }
            if(event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(window)) {
                done = true;
            }
        }
    }

    inline void start_new_frame() {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    inline void connecting_window() {
        static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
        ImGui::Begin("ESP32_AD5933", NULL, flags);                          // Create a window called "Hello, world!" and append into it.
        static uint8_t dot_count = 0;
        switch(dot_count) {
            case 0:
                ImGui::Text("Connecting.");
                break;
            case 1:
                ImGui::Text("Connecting..");
                break;
            case 2:
                ImGui::Text("Connecting...");
                break;
            default:
                ImGui::Text("Connecting");
                break;
        }
        dot_count++;
        if(dot_count > 2) {
            dot_count = 0;
        }
        ImGui::End();
    }
}

namespace ImGuiSDL {
    // Demos - unused for now
    inline void periodic_temperature_measurement_demo_window(std::optional<ESP32_AD5933> &esp32_ad5933) {
        static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
        ImGui::Begin("Periodic Temperature Measurement Demo", NULL, flags);                          // Create a window called "Hello, world!" and append into it.
        static bool toggle_capture;
        bool toggle_capture_prev = toggle_capture;
        ImGui::Checkbox("Toggle Temperature Measurement: ", &toggle_capture);
        if(esp32_ad5933.has_value() && (toggle_capture != toggle_capture_prev)) {
            if(toggle_capture == true) {
                esp32_ad5933.value().subscribe_to_body_composition_measurement_indicate();
            } else {
                esp32_ad5933.value().unsubscribe_from_body_composistion_measurement();
            }
        }
        ImGui::SameLine();
        if(esp32_ad5933.has_value()) {
            ImGui::Text(esp32_ad5933.value().temp_payload.value_or("0xFFFF'FFFF").c_str());
        }
        ImGui::End();
    }
}

namespace ImGuiSDL {
    // Related to measurement_window  
    static auto freq_start_array_init_lambda = [](AD5933_Config &config) -> std::array<char, 7> {
        std::array<char, 7> ret_val { 0x00 };
        const auto ret_str = std::to_string(static_cast<uint32_t>(config.get_start_freq()));
        std::copy(ret_str.begin(), ret_str.end(), ret_val.begin());
        return ret_val;
    };

    static auto freq_inc_array_init_lambda = [](AD5933_Config &config) -> std::array<char, 7> {
        std::array<char, 7> ret_val { 0x00 };
        const auto ret_str = std::to_string(static_cast<uint32_t>(config.get_inc_freq()));
        std::copy(ret_str.begin(), ret_str.end(), ret_val.begin());
        return ret_val;
    };

    static auto num_of_inc_array_init_lambda = [](AD5933_Config &config) -> std::array<char, 4> {
        std::array<char, 4> ret_val { 0x00 };
        const auto ret_str = std::to_string(config.get_num_of_inc().value);
        std::copy(ret_str.begin(), ret_str.end(), ret_val.begin());
        return ret_val;
    };

    static auto settling_time_cycles_num_array_init_lambda = [](AD5933_Config &config) -> std::array<char, 4> {
        std::array<char, 4> ret_val { 0x00 };
        const auto ret_str = std::to_string(config.get_settling_time_cycles_number().value);
        std::copy(ret_str.begin(), ret_str.end(), ret_val.begin());
        return ret_val;
    };

    static auto sysclk_freq_array_init_lambda = [](AD5933_Config &config) -> std::array<char, 9> {
        std::array<char, 9> ret_val { 0x00 };
        const auto ret_str = std::to_string(config.active_sysclk_freq);
        std::copy(ret_str.begin(), ret_str.end(), ret_val.begin());
        return ret_val;
    };

    //std::vector<AD5933_CalibrationData> calibration_data;
    //std::vector<AD5933_MeasurementData> measurement_data;

    class MeasurementWindowCaptures {
    public:
        AD5933_Config config { AD5933_Config::get_default() };

        std::array<char, 7> freq_start { freq_start_array_init_lambda(config) };
        std::array<char, 7> freq_inc { freq_inc_array_init_lambda(config) };
        std::array<char, 4> num_of_inc { num_of_inc_array_init_lambda(config)  };
        std::array<char, 4> settling_time_cycles_num { settling_time_cycles_num_array_init_lambda(config) };
        int settling_time_cycles_multiplier_combo = 0;
        int voltage_range_combo = 0;
        int pga_gain_combo = 1;
        int sysclk_src_combo = 1;

        std::array<char, 9> sysclk_freq { sysclk_freq_array_init_lambda(config) };

        MeasurementWindowCaptures() = default;

        void update_config() {
            auto stoul_lambda = [](const std::string &num_str) -> std::optional<unsigned long> {
                try {
                    size_t pos;
                    if(num_str.empty()) {
                        return std::nullopt;
                    }
                    unsigned long number = std::stoul(num_str, &pos);

                    if(pos < std::strlen(num_str.c_str())) {
                        std::cout << "Conversion failed. Not the entire string was converted." << std::endl;
                        return std::nullopt; // Return empty optional if conversion is incomplete
                    } else {
                        return number;
                    }
                } catch (const std::out_of_range& e) {
                    std::cout << "Exception caught: " << e.what() << std::endl;
                    // Handle the out_of_range exception here
                } catch (const std::invalid_argument& e) {
                    std::cout << "Invalid argument exception caught: " << e.what() << std::endl;
                    // Handle the invalid_argument exception here
                }
                    return std::nullopt; // Return empty optional in case of exceptions
            };
            config = AD5933_Config {
                config.get_command(),
                std::next(ControlHB_OutputVoltageRangeOrMaskStringMap.begin(), voltage_range_combo)->first,
                std::next(ControlHB_PGA_GainOrMaskStringMap.begin(), pga_gain_combo)->first,
                std::next(ControlLB_SYSCLK_SRC_OrMaskStringMap.begin(), sysclk_src_combo)->first,
                stoul_lambda(std::string(freq_start.begin(), freq_start.end())).value_or(static_cast<uint32_t>(config.get_start_freq())),
                stoul_lambda(std::string(freq_inc.begin(), freq_inc.end())).value_or(static_cast<uint32_t>(config.get_inc_freq())),
                uint9_t { static_cast<uint16_t>(stoul_lambda(std::string(num_of_inc.begin(), num_of_inc.end())).value_or(config.get_num_of_inc().value)) },
                uint9_t { static_cast<uint16_t>(stoul_lambda(std::string(settling_time_cycles_num.begin(), settling_time_cycles_num.end())).value_or(config.get_settling_time_cycles_number().value)) },
                std::next(SettlingTimeCyclesMultiplierOrMaskStringMap.begin(), settling_time_cycles_multiplier_combo)->first,
                config.calibration_impedance
            };
            sysclk_freq = sysclk_freq_array_init_lambda(config);
        }
    };
    std::atomic<std::shared_ptr<MeasurementWindowCaptures>> measure_captures { std::make_shared<MeasurementWindowCaptures>() };
}

namespace ImGuiSDL {
    // Related to dbg_registers_window   
    template<size_t N>
    class DebugReadWriteRegistersCaptures {
    public:
        std::array<char, N> ctrl_HB { 0x00 };
        std::array<char, N> ctrl_LB { 0x00 };
        std::array<char, N> freq_start_HB { 0x00 };
        std::array<char, N> freq_start_MB { 0x00 };
        std::array<char, N> freq_start_LB { 0x00 };
        std::array<char, N> freq_inc_HB { 0x00 };
        std::array<char, N> freq_inc_MB { 0x00 };
        std::array<char, N> freq_inc_LB { 0x00 };
        std::array<char, N> num_of_inc_HB { 0x00 };
        std::array<char, N> num_of_inc_LB { 0x00 };
        std::array<char, N> num_of_settling_time_cycles_HB { 0x00 };
        std::array<char, N> num_of_settling_time_cycles_LB { 0x00 };
        std::array<char, N> status_capture { 0x00 };
        std::array<char, N> temp_data_HB { 0x00 };
        std::array<char, N> temp_data_LB { 0x00 };
        std::array<char, N> real_data_HB { 0x00 };
        std::array<char, N> real_data_LB { 0x00 };
        std::array<char, N> imag_data_HB { 0x00 };
        std::array<char, N> imag_data_LB { 0x00 };
        std::array<std::array<char, N>*, 19> all { 
            &ctrl_HB,
            &ctrl_LB,
            &freq_start_HB,
            &freq_start_MB,
            &freq_start_LB,
            &freq_inc_HB,
            &freq_inc_MB,
            &freq_inc_LB,
            &num_of_inc_HB,
            &num_of_inc_LB,
            &num_of_settling_time_cycles_HB,
            &num_of_settling_time_cycles_LB,
            &status_capture,
            &temp_data_HB,
            &temp_data_LB,
            &real_data_HB,
            &real_data_LB,
            &imag_data_HB,
            &imag_data_LB
        };

        AD5933_Config config {};
        AD5933_Data data {};

        DebugReadWriteRegistersCaptures() = default;

        DebugReadWriteRegistersCaptures(const DebugReadWriteRegistersCaptures &other) {
            auto empty_default = DebugReadWriteRegistersCaptures();
            all = std::array<std::array<char, N>*, 19>{
                &ctrl_HB,
                &ctrl_LB,
                &freq_start_HB,
                &freq_start_MB,
                &freq_start_LB,
                &freq_inc_HB,
                &freq_inc_MB,
                &freq_inc_LB,
                &num_of_inc_HB,
                &num_of_inc_LB,
                &num_of_settling_time_cycles_HB,
                &num_of_settling_time_cycles_LB,
                &status_capture,
                &temp_data_HB,
                &temp_data_LB,
                &real_data_HB,
                &real_data_LB,
                &imag_data_HB,
                &imag_data_LB
            };
            assert(all[0] == &ctrl_HB);
            for(size_t i = 0; i < all.size(); i++) {
                *(all[i]) = *(other.all[i]);
            }
            config = other.config;
            data = other.data;
        }

        void update_config() {
            auto stoul_lambda = [](const std::array<char, 5>& hex_array) -> std::optional<uint8_t> {
                try {
                    // Extract the numeric value from the string
                    uint8_t result = static_cast<uint8_t>(std::stoul(std::string { hex_array.begin(), hex_array.end() }, nullptr, 16)); // Convert hexadecimal string to unsigned long
                    return result;
                } catch (...) {
                    return std::nullopt; // Return empty optional on conversion failure
                }
            };
            const std::array<uint8_t, 12> tmp_raw_array {
                stoul_lambda(ctrl_HB).value_or(static_cast<uint8_t>(config.control_HB.HB.to_ulong())),
                stoul_lambda(ctrl_LB).value_or(static_cast<uint8_t>(config.control_LB.HB.to_ulong())),
                stoul_lambda(freq_start_HB).value_or(static_cast<uint8_t>(config.start_freq.HB.to_ulong())),
                stoul_lambda(freq_start_MB).value_or(static_cast<uint8_t>(config.start_freq.MB.to_ulong())),
                stoul_lambda(freq_start_LB).value_or(static_cast<uint8_t>(config.start_freq.MB.to_ulong())),
                stoul_lambda(freq_inc_HB).value_or(static_cast<uint8_t>(config.inc_freq.HB.to_ulong())),
                stoul_lambda(freq_inc_MB).value_or(static_cast<uint8_t>(config.inc_freq.MB.to_ulong())),
                stoul_lambda(freq_inc_LB).value_or(static_cast<uint8_t>(config.inc_freq.LB.to_ulong())),
                stoul_lambda(num_of_inc_HB).value_or(static_cast<uint8_t>(config.num_of_inc.HB.to_ulong())),
                stoul_lambda(num_of_inc_LB).value_or(static_cast<uint8_t>(config.num_of_inc.LB.to_ulong())),
                stoul_lambda(num_of_settling_time_cycles_HB).value_or(static_cast<uint8_t>(config.settling_time_cycles.HB.to_ulong())),
                stoul_lambda(num_of_settling_time_cycles_LB).value_or(static_cast<uint8_t>(config.settling_time_cycles.LB.to_ulong())),
            };
            config = AD5933_Config { tmp_raw_array };
        }
    };

    class HexDebugReadWriteRegistersCaptures : public DebugReadWriteRegistersCaptures<5> {
    public:
        HexDebugReadWriteRegistersCaptures() :
            DebugReadWriteRegistersCaptures {}
        {}
    };

    class BinDebugReadWriteRegistersCaptures : public DebugReadWriteRegistersCaptures<12> {

    };

    std::atomic<std::shared_ptr<HexDebugReadWriteRegistersCaptures >> hex_dbg_rw_captures { std::make_shared<HexDebugReadWriteRegistersCaptures>() };
    std::atomic<int> debug_window_enable { 0 };

    void update_hex_dbg_rw_captures(const std::string &data) {
        //auto tmp_hex_dbg_rw_captures = *(hex_dbg_rw_captures.load());
        HexDebugReadWriteRegistersCaptures tmp_hex_dbg_rw_captures {};

        std::array<std::string, 19> oss_array {};
        for(size_t i = 0; i < data.size(); i++) {
            std::ostringstream temp_oss;
            temp_oss << "0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(data[i]));
            oss_array[i] = temp_oss.str();
        }

        size_t i = 0;
        assert(tmp_hex_dbg_rw_captures.all[0] == &tmp_hex_dbg_rw_captures.ctrl_HB);
        for(const auto &oss_content: oss_array) {
            std::array<char, 5> temp_array{}; // Initialize an array of chars with size 5
            std::copy(oss_content.begin(), oss_content.begin() + std::min(oss_content.size(), temp_array.size()), temp_array.begin());
            std::copy(temp_array.begin(), temp_array.end(), tmp_hex_dbg_rw_captures.all[i]->begin());
            i++;
        }

        std::array<uint8_t, 12> config_data_array;
        std::transform(
            data.begin(),
            data.begin() + config_data_array.size(),
            config_data_array.begin(), 
            [](char c) { 
                return static_cast<uint8_t>(c);
            }
        );
        tmp_hex_dbg_rw_captures.config = AD5933_Config { config_data_array };

        std::array<uint8_t, 7> data_data_array;
        std::transform(
            data.begin() + config_data_array.size(),
            data.end(),
            data_data_array.begin(), 
            [](char c) { 
                return static_cast<uint8_t>(c);
            }
        );
        tmp_hex_dbg_rw_captures.data = AD5933_Data { data_data_array };
        const auto try_ret = std::make_shared<HexDebugReadWriteRegistersCaptures>(tmp_hex_dbg_rw_captures);
        hex_dbg_rw_captures.store(std::make_shared<HexDebugReadWriteRegistersCaptures>(tmp_hex_dbg_rw_captures));
    }

    void send_debug_start_req_thread_cb(std::optional<ESP32_AD5933> &esp32_ad5933) {
        esp32_ad5933.value().send(std::string(MagicPackets::Debug::Command::start.begin(), MagicPackets::Debug::Command::start.end()));
    }

    void send_debug_end_req_thread_cb(std::optional<ESP32_AD5933> &esp32_ad5933) {
        esp32_ad5933.value().send(std::string(MagicPackets::Debug::Command::end.begin(), MagicPackets::Debug::Command::end.end()));
    }
   
    void send_dump_all_registers_req_thread_cb(std::optional<ESP32_AD5933> &esp32_ad5933) {
        esp32_ad5933.value().send(std::string(MagicPackets::Debug::Command::dump_all_registers.begin(), MagicPackets::Debug::Command::dump_all_registers.end()));
        const auto rx_register_info = esp32_ad5933.value().rx_payload.read();
        esp32_ad5933.value().rx_payload.clean();

        esp32_ad5933.value().print_mtu();
        for(size_t i = 0; i < rx_register_info.size(); i++) {
            std::printf("Register[%02zu]: 0x%02X\n", i, static_cast<uint8_t>(rx_register_info[i]));
        }
        update_hex_dbg_rw_captures(std::string(rx_register_info.begin(), rx_register_info.begin() + 19));
    }

    void dbg_registers_thread_cb() {
        while(debug_window_enable.load() > 0) {
            debug_window_enable.wait(debug_window_enable);
            if(debug_window_enable.load() == 2) {
                std::cout << "ImGuiSDL: Loading rx_payload content\n";
                debug_window_enable.store(1);
            }
        }

        debug_window_enable.store(0);
        std::cout << "ImGuiSDL: ending dbg_registers_thread_cb\n";
    }

    void program_all_registers_thread_cb(std::array<uint8_t, 12> raw_message, std::optional<ESP32_AD5933> &esp32_ad5933) {
        auto buf_to_send = MagicPackets::Debug::Command::program_all_registers;
        std::copy(raw_message.begin(), raw_message.end(), buf_to_send.begin());
        const auto tmp_dbg_var = std::string(buf_to_send.begin(), buf_to_send.end());
        esp32_ad5933.value().send(tmp_dbg_var);
    }

    void dbg_program_all_registers_thread_cb(std::array<uint8_t, 12> raw_message, std::optional<ESP32_AD5933> &esp32_ad5933) {
        program_all_registers_thread_cb(raw_message, esp32_ad5933);

        std::thread send_dump_all_registers_thread(send_dump_all_registers_req_thread_cb, std::ref(esp32_ad5933));
        send_dump_all_registers_thread.join();
    }

    void control_command_thread_cb(const ControlHB::CommandOrMask command, std::optional<ESP32_AD5933> &esp32_ad5933) {
        auto buf_to_send = MagicPackets::Debug::Command::control_HB_command;
        buf_to_send[0] = static_cast<uint8_t>(command);
        esp32_ad5933.value().send(std::string(buf_to_send.begin(), buf_to_send.end()));
        send_dump_all_registers_req_thread_cb(esp32_ad5933);
    }

    static const auto default_config = AD5933_Config::get_default();
    void debug_registers_window(std::optional<ESP32_AD5933> &esp32_ad5933) {
        static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
        ImGui::Begin("Register Debug", NULL, flags);                          // Create a window called "Hello, world!" and append into it.

        auto tmp_hex_dbg_rw_captures = *(hex_dbg_rw_captures.load());

        static bool debug_started = false;
        if(debug_started == false && ImGui::Button("Debug Start")) {
            debug_started = true;
            std::thread debug_start_thread(send_debug_start_req_thread_cb, std::ref(esp32_ad5933));
            debug_start_thread.detach();
        }

        if(debug_started == true) {
            ImGui::SameLine();
            if(ImGui::Button("Dump All Registers")) {
                std::thread dump_all_registers_thread(send_dump_all_registers_req_thread_cb, std::ref(esp32_ad5933));
                dump_all_registers_thread.detach();
            }; ImGui::SameLine();

            if(ImGui::Button("Program Read/Write Registers")) {
                std::thread dbg_program_all_registers_thread(
                    dbg_program_all_registers_thread_cb,
                    tmp_hex_dbg_rw_captures.config.get_ad5933_config_message_raw(),
                    std::ref(esp32_ad5933)
                );
                dbg_program_all_registers_thread.detach();
            }; ImGui::SameLine();

            if(ImGui::Button("Debug End")) {
                std::thread debug_end_thread(send_debug_end_req_thread_cb, std::ref(esp32_ad5933));
                debug_end_thread.detach();
                debug_started = false;
            }
        }

        ImGui::Separator();

        ImGui::Text("Decoded Register Values");
        ImGui::Separator();

        ImGui::Text("ControlHB:");
        ImGui::Text("\tControlHB Command: %s", ControlHB_CommandOrMaskStringMap.at(tmp_hex_dbg_rw_captures.config.get_command()));
        ImGui::Text("\tExcitation Output Voltage Range: %s", ControlHB_OutputVoltageRangeOrMaskStringMap.at(tmp_hex_dbg_rw_captures.config.get_voltage_range()));
        ImGui::Text("\tPGA Gain: %s", ControlHB_PGA_GainOrMaskStringMap.at(tmp_hex_dbg_rw_captures.config.get_pga_gain()));
        ImGui::Separator();

        ImGui::Text("ControlLB:");
        ImGui::Text("\tReset: %s", tmp_hex_dbg_rw_captures.config.get_reset() == true ? "true" : "false");
        ImGui::Text("\tSystem Clock Source: %s", ControlLB_SYSCLK_SRC_OrMaskStringMap.at(tmp_hex_dbg_rw_captures.config.get_sysclk_src())); 
        ImGui::Separator();

        ImGui::Text("Start Frequency: %f", tmp_hex_dbg_rw_captures.config.get_start_freq());
        ImGui::Text("Frequency Increment: %f", tmp_hex_dbg_rw_captures.config.get_inc_freq());
        ImGui::Text("Number of Increments: %u", tmp_hex_dbg_rw_captures.config.get_num_of_inc().value);
        ImGui::Text("Number of Settling Time Cycles: %u", tmp_hex_dbg_rw_captures.config.get_settling_time_cycles_number().value);
        ImGui::Text("Settling Time Cycles Multiplier: %s", SettlingTimeCyclesMultiplierOrMaskStringMap.at(tmp_hex_dbg_rw_captures.config.get_settling_time_cycles_multiplier()));
        ImGui::Separator();

        ImGui::Text("Status: %s", STATUS_OrMaskStringMap.at(tmp_hex_dbg_rw_captures.data.get_status()));
        ImGui::Separator();

        ImGui::Text("Temperature: %f °C", tmp_hex_dbg_rw_captures.data.get_temperature());

        ImGui::InputText("Real Part",
            std::to_string(tmp_hex_dbg_rw_captures.data.get_real_part()).data(),
            std::to_string(tmp_hex_dbg_rw_captures.data.get_real_part()).size(),
            ImGuiInputTextFlags_ReadOnly
        );

        ImGui::InputText("Imaginary Part",
            std::to_string(tmp_hex_dbg_rw_captures.data.get_imag_part()).data(),
            std::to_string(tmp_hex_dbg_rw_captures.data.get_imag_part()).size(),
            ImGuiInputTextFlags_ReadOnly
        );

        ImGui::InputText("Raw Magnitude",
            std::to_string(tmp_hex_dbg_rw_captures.data.get_raw_magnitude()).data(),
            std::to_string(tmp_hex_dbg_rw_captures.data.get_raw_magnitude()).size(),
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
            float_to_str_lambda(tmp_hex_dbg_rw_captures.data.get_raw_phase()).data(),
            float_to_str_lambda(tmp_hex_dbg_rw_captures.data.get_raw_phase()).size(),
            ImGuiInputTextFlags_ReadOnly
        );

        ImGui::Separator();

        ImGui::Text("Read/Write Registers");
        {
            ImGui::Separator();
            ImGui::InputText("CTRL_HB (0x80)", tmp_hex_dbg_rw_captures.ctrl_HB.data(), tmp_hex_dbg_rw_captures.ctrl_HB.size());
            ImGui::InputText("CTRL_LB (0x81)", tmp_hex_dbg_rw_captures.ctrl_LB.data(), tmp_hex_dbg_rw_captures.ctrl_LB.size());
            ImGui::Separator();
            ImGui::InputText("FREQ_START_HB (0x82)", tmp_hex_dbg_rw_captures.freq_start_HB.data(), tmp_hex_dbg_rw_captures.freq_start_HB.size());
            ImGui::InputText("FREQ_START_MB (0x83)", tmp_hex_dbg_rw_captures.freq_start_MB.data(), tmp_hex_dbg_rw_captures.freq_start_MB.size());
            ImGui::InputText("FREQ_START_LB (0x84)", tmp_hex_dbg_rw_captures.freq_start_LB.data(), tmp_hex_dbg_rw_captures.freq_start_LB.size());
            ImGui::Separator();
            ImGui::InputText("FREQ_INC_HB (0x85)", tmp_hex_dbg_rw_captures.freq_inc_HB.data(), tmp_hex_dbg_rw_captures.freq_inc_HB.size());
            ImGui::InputText("FREQ_INC_MB (0x86)", tmp_hex_dbg_rw_captures.freq_inc_MB.data(), tmp_hex_dbg_rw_captures.freq_inc_MB.size());
            ImGui::InputText("FREQ_INC_LB (0x87)", tmp_hex_dbg_rw_captures.freq_inc_LB.data(), tmp_hex_dbg_rw_captures.freq_inc_LB.size());
            ImGui::Separator();
            ImGui::InputText("NUM_OF_INC_HB (0x88)", tmp_hex_dbg_rw_captures.num_of_inc_HB.data(), tmp_hex_dbg_rw_captures.num_of_inc_HB.size());
            ImGui::InputText("NUM_OF_INC_LB (0x89)", tmp_hex_dbg_rw_captures.num_of_inc_LB.data(), tmp_hex_dbg_rw_captures.num_of_inc_LB.size());
            ImGui::Separator();
            ImGui::InputText("NUM_OF_SETTLING_TIME_CYCLES_HB (0x8A)", tmp_hex_dbg_rw_captures.num_of_settling_time_cycles_HB.data(), tmp_hex_dbg_rw_captures.num_of_settling_time_cycles_HB.size());
            ImGui::InputText("NUM_OF_SETTLING_TIME_CYCLES_LB (0x8B)", tmp_hex_dbg_rw_captures.num_of_settling_time_cycles_LB.data(), tmp_hex_dbg_rw_captures.num_of_settling_time_cycles_LB.size());

            ImGui::Separator();

            {
            ImGui::Text("Send Control Register Command Controls");
            if(ImGui::Button("Power-down mode")) {
                std::thread(control_command_thread_cb, ControlHB::CommandOrMask::POWER_DOWN_MODE, std::ref(esp32_ad5933)).detach();
            } ImGui::SameLine();
                if(ImGui::Button("Standby mode")) {
                    std::thread(control_command_thread_cb, ControlHB::CommandOrMask::STANDBY_MODE, std::ref(esp32_ad5933)).detach();
                } ImGui::SameLine();
                if(ImGui::Button("No operation (NOP_0)")) {
                    std::thread(control_command_thread_cb, ControlHB::CommandOrMask::NOP_0, std::ref(esp32_ad5933)).detach();
                }

            if(ImGui::Button("Measure temperature")) {
                std::thread(control_command_thread_cb, ControlHB::CommandOrMask::MEASURE_TEMPERATURE, std::ref(esp32_ad5933)).detach();
            }

            if(ImGui::Button("Initialize with start frequency")) {
                std::thread(control_command_thread_cb, ControlHB::CommandOrMask::INITIALIZE_WITH_START_FREQUENCY, std::ref(esp32_ad5933)).detach();
            } ImGui::SameLine();
                if(ImGui::Button("Start frequency sweep")) {
                    std::thread(control_command_thread_cb, ControlHB::CommandOrMask::START_FREQUENCY_SWEEP, std::ref(esp32_ad5933)).detach();
                } ImGui::SameLine();
                if(ImGui::Button("Increment frequency")) {
                    std::thread(control_command_thread_cb, ControlHB::CommandOrMask::INCREMENT_FREQUENCY, std::ref(esp32_ad5933)).detach();
                } ImGui::SameLine();
                if(ImGui::Button("Repeat frequency")) {
                    std::thread(control_command_thread_cb, ControlHB::CommandOrMask::REPEAT_FREQUENCY, std::ref(esp32_ad5933)).detach();
                }
            }

            ImGui::Separator();

            ImGui::Text("Special Status Register");
            ImGui::InputText("STATUS (0x8F)", tmp_hex_dbg_rw_captures.status_capture.data(), tmp_hex_dbg_rw_captures.status_capture.size(), ImGuiInputTextFlags_ReadOnly);

            ImGui::Separator();

            ImGui::Text("Read-only Data Registers");
            ImGui::InputText("TEMP_DATA_HB (0x92)", tmp_hex_dbg_rw_captures.temp_data_HB.data(), tmp_hex_dbg_rw_captures.temp_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::InputText("TEMP_DATA_LB (0x93)", tmp_hex_dbg_rw_captures.temp_data_LB.data(), tmp_hex_dbg_rw_captures.temp_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::Separator();
            ImGui::InputText("REAL_DATA_HB (0x94)", tmp_hex_dbg_rw_captures.real_data_HB.data(), tmp_hex_dbg_rw_captures.real_data_HB.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::InputText("REAL_DATA_LB (0x95)", tmp_hex_dbg_rw_captures.real_data_LB.data(), tmp_hex_dbg_rw_captures.real_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::Separator();
            ImGui::InputText("IMAG_DATA_HB (0x96)", tmp_hex_dbg_rw_captures.imag_data_HB.data(), tmp_hex_dbg_rw_captures.imag_data_HB.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::InputText("IMAG_DATA_LB (0x97)", tmp_hex_dbg_rw_captures.imag_data_LB.data(), tmp_hex_dbg_rw_captures.imag_data_LB.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::Separator();
        }

        tmp_hex_dbg_rw_captures.update_config();
        hex_dbg_rw_captures.store(std::make_shared<HexDebugReadWriteRegistersCaptures>(tmp_hex_dbg_rw_captures));
        ImGui::End();
    }
}

namespace ImGuiSDL {
    std::vector<AD5933_MeasurementData> measurement_data;
    std::vector<AD5933_CalibrationData> calibration_data;

    void calibrate_cb(bool &calibrating, bool &calibrated, std::atomic<float> &progress_bar_fraction, std::optional<ESP32_AD5933> &esp32_ad5933) {
        esp32_ad5933.value().send(std::string(
            MagicPackets::FrequencySweep::Command::start.begin(),
            MagicPackets::FrequencySweep::Command::start.end()
        ));

        const uint16_t wished_size = measure_captures.load()->config.get_num_of_inc().value + 1;
        const float progress_bar_step = 1.0f / static_cast<float>(wished_size);

        calibration_data.clear();
        calibration_data.reserve(wished_size);

        do {
            const auto rx_payload = esp32_ad5933.value().rx_payload.read();
            esp32_ad5933.value().rx_payload.clean();
            AD5933_CalibrationData tmp_calibration_data {
                static_cast<uint8_t>(rx_payload[0]),
                static_cast<uint8_t>(rx_payload[1]),
                static_cast<uint8_t>(rx_payload[2]),
                static_cast<uint8_t>(rx_payload[3]),
            };

            calibration_data.push_back(tmp_calibration_data);
            progress_bar_fraction.fetch_add(progress_bar_step);
        } while(calibration_data.size() != wished_size);

        calibrating = false;
        calibrated = true;
    }

    void start_sweep_cb(bool &sweeping, bool &sweeped, std::atomic<float> &progress_bar_fraction, std::optional<ESP32_AD5933> &esp32_ad5933, bool &periodically_sweeping) {
        do {
            esp32_ad5933.value().send(std::string(
                MagicPackets::FrequencySweep::Command::start.begin(),
                MagicPackets::FrequencySweep::Command::start.end()
            ));

            const uint16_t wished_size = measure_captures.load()->config.get_num_of_inc().value + 1;
            const float progress_bar_step = 1.0f / static_cast<float>(wished_size);
            std::vector<AD5933_MeasurementData> tmp_measurement_data_vector;
            tmp_measurement_data_vector.reserve(wished_size);
            progress_bar_fraction.store(0.0);

            do {
                const auto rx_payload = esp32_ad5933.value().rx_payload.read();
                esp32_ad5933.value().rx_payload.clean();
                AD5933_MeasurementData tmp_measurement_data {
                    static_cast<uint8_t>(rx_payload[0]),
                    static_cast<uint8_t>(rx_payload[1]),
                    static_cast<uint8_t>(rx_payload[2]),
                    static_cast<uint8_t>(rx_payload[3]),
                };

                tmp_measurement_data_vector.push_back(tmp_measurement_data);
                progress_bar_fraction.fetch_add(progress_bar_step);
            } while(tmp_measurement_data_vector.size() != wished_size);

            measurement_data.clear();
            measurement_data = tmp_measurement_data_vector;

            sweeped = true;
        } while(periodically_sweeping);

        sweeping = false;
    }
}

namespace ImGuiSDL {
    inline void measurement_window(ImVec2 &measurement_window_size, std::optional<ESP32_AD5933> &esp32_ad5933, bool &plot_calibration_window_enable, bool &plot_freq_sweep_window_enable) {
        static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
        ImGui::Begin("Measurement", NULL, flags);                          // Create a window called "Hello, world!" and append into it.
        if(ImGui::Button("Toggle Register Debug Window") == true) {
            if(debug_window_enable.load() == false) {
                measurement_window_size.x /= 2;
                debug_window_enable.store(1);
                std::thread dbg_registers_thread(dbg_registers_thread_cb);
                dbg_registers_thread.detach();
            } else {
                measurement_window_size = ImGui::GetIO().DisplaySize;
                debug_window_enable.store(0);
            }
            debug_window_enable.notify_all();
        }

        auto tmp_measure_captures = *(measure_captures.load());

        static bool calibrating = false;
        static bool calibrated = false;
        static bool sweeping = false;
        static bool sweeped = false;

        const ImGuiInputTextFlags input_flags = (calibrating == true || sweeping == true) ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None;
        ImGui::Separator(); 
        ImGui::Text("Sweep Parameters");

        ImGui::Separator();

        ImGui::InputText("Start Frequency", tmp_measure_captures.freq_start.data(), tmp_measure_captures.freq_start.size(), input_flags);
        ImGui::InputText("Increment Frequency", tmp_measure_captures.freq_inc.data(), tmp_measure_captures.freq_inc.size(), input_flags);

        ImGui::InputText("Number of Increments", tmp_measure_captures.num_of_inc.data(), tmp_measure_captures.num_of_inc.size(), input_flags);

        ImGui::Separator();

        ImGui::InputInt("Calibration impedance", &tmp_measure_captures.config.calibration_impedance, 0, 0, input_flags);

        ImGui::Separator(); 

        ImGui::Combo("Output Excitation Voltage Range", &tmp_measure_captures.voltage_range_combo, "2 Vppk""\0""200 mVppk""\0""400 mVppk""\0""1 Vppk");

        ImGui::Separator();

        ImGui::Combo("PGA Gain", &tmp_measure_captures.pga_gain_combo, "x5\0x1");

        ImGui::Separator();

        ImGui::InputText("Number of Settling Time Cycles", tmp_measure_captures.settling_time_cycles_num.data(), tmp_measure_captures.settling_time_cycles_num.size(), input_flags);
        ImGui::Combo("Settling Time Cycles Multiplier", &tmp_measure_captures.settling_time_cycles_multiplier_combo, "x1\0x2\0x4");

        ImGui::Separator();

        ImGui::Combo("System Clock Source", &tmp_measure_captures.sysclk_src_combo, "Internal\0External");
        ImGui::InputText("System Clock Frequency", tmp_measure_captures.sysclk_freq.data(), tmp_measure_captures.sysclk_freq.size(), ImGuiInputTextFlags_ReadOnly);

        ImGui::Separator(); 

        if(ImGui::Button("Program Device Registers")) {
            std::thread(
                [tmp_measure_captures, &esp32_ad5933]() {
                    send_debug_start_req_thread_cb(esp32_ad5933);
                    program_all_registers_thread_cb(tmp_measure_captures.config.get_ad5933_config_message_raw(), esp32_ad5933);
                    send_debug_end_req_thread_cb(esp32_ad5933);
                }
            ).detach();
        }

        static std::atomic<float> calibrating_progress_bar_fraction { 0.0f };
        if(calibrating == false && sweeping == false) {
            if(ImGui::Button("Calibrate")) {
                calibrating = true;
                calibrated = false;
                calibrating_progress_bar_fraction.store(0.0f);
                std::thread(calibrate_cb,
                    std::ref(calibrating),
                    std::ref(calibrated),
                    std::ref(calibrating_progress_bar_fraction),
                    std::ref(esp32_ad5933)
                ).detach();
            }
        } else if(sweeping == false) {
            ImGui::Spinner("Calibrating", 10.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
            ImGui::ProgressBar(calibrating_progress_bar_fraction.load());
        }

        static std::atomic<float> sweeping_progress_bar_fraction { 0.0f };
        if(calibrated == true) {
            ImGui::SameLine(); if(ImGui::Button("View Calibration Data")) {
                plot_calibration_window_enable = true;
            }
            static bool periodically_sweeping = false;
            ImGui::Checkbox("Periodic Sweep", &periodically_sweeping);
            if(sweeping == false) {
                if(ImGui::Button("Start Sweep")) {
                    sweeping = true;
                    sweeped = false;
                    sweeping_progress_bar_fraction.store(0.0f);
                    std::thread(start_sweep_cb,
                        std::ref(sweeping),
                        std::ref(sweeped),
                        std::ref(sweeping_progress_bar_fraction),
                        std::ref(esp32_ad5933),
                        std::ref(periodically_sweeping)
                    ).detach();
                }
            } else if(calibrating == false) {
                ImGui::Spinner("Measuring", 10.0f, 2.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                ImGui::ProgressBar(sweeping_progress_bar_fraction.load());
            }
        }

        if(sweeped == true) {
            ImGui::SameLine(); if(ImGui::Button("View Frequency Sweep Data")) {
                plot_freq_sweep_window_enable = true;
            }
        }

        ImGui::Separator(); 

        ImGui::End();
        tmp_measure_captures.update_config();
        measure_captures.store(std::make_shared<MeasurementWindowCaptures>(tmp_measure_captures));
    }
}

namespace ImGuiSDL {
    void calibration_raw_data_plots() {
        const auto start_freq = measure_captures.load()->config.get_start_freq();
        const auto inc_freq = measure_captures.load()->config.get_inc_freq();
        std::vector<double> frequency_vector(calibration_data.size());
        std::generate(frequency_vector.begin(), frequency_vector.end(), [start_freq, inc_freq, n = 0.0] () mutable { return static_cast<double>(start_freq + ((n++) * inc_freq)); });

        if(ImPlot::BeginPlot("Calibration Raw Real Data")) {
            ImPlot::SetupAxes("f [Hz]", "REAL_DATA");
            std::vector<double> real_data_vector(calibration_data.size());
            std::transform(calibration_data.begin(), calibration_data.end(), real_data_vector.begin(), [](AD5933_CalibrationData &e) { return static_cast<double>(e.get_real_part()); });
            ImPlot::PlotLine("REAL_DATA [1/Ohm]", frequency_vector.data(), real_data_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
         if(ImPlot::BeginPlot("Calibration Raw Imag Data")) {
            ImPlot::SetupAxes("f [Hz]", "IMAG_DATA");
            std::vector<double> imag_data_vector(calibration_data.size());
            std::transform(calibration_data.begin(), calibration_data.end(), imag_data_vector.begin(), [](AD5933_CalibrationData &e) { return static_cast<double>(e.get_imag_part()); });
            ImPlot::PlotLine("IMAG_DATA [1/Ohm]", frequency_vector.data(), imag_data_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
    }

    void calibration_calculated_data_plots() {
        const auto start_freq = measure_captures.load()->config.get_start_freq();
        const auto inc_freq = measure_captures.load()->config.get_inc_freq();
        std::vector<double> frequency_vector(calibration_data.size());
        std::generate(frequency_vector.begin(), frequency_vector.end(), [start_freq, inc_freq, n = 0.0] () mutable { return static_cast<double>(start_freq + ((n++) * inc_freq)); });

        if(ImPlot::BeginPlot("Calibration Calculated Magnitude")) {
            ImPlot::SetupAxes("f [Hz]", "RAW_MAGNITUDE");
            std::vector<double> raw_magnitude_vector(calibration_data.size());
            std::transform(calibration_data.begin(), calibration_data.end(), raw_magnitude_vector.begin(), [](AD5933_CalibrationData &e) { return static_cast<double>(e.get_raw_magnitude()); });
            ImPlot::PlotLine("RAW_MAGNITUDE [1/Ohm]", frequency_vector.data(), raw_magnitude_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
         if(ImPlot::BeginPlot("Calibration Calculated Phase")) {
            ImPlot::SetupAxes("f [Hz]", "RAW_PHASE");
            std::vector<double> raw_phase_vector(calibration_data.size());
            std::transform(calibration_data.begin(), calibration_data.end(), raw_phase_vector.begin(), [](AD5933_CalibrationData &e) { return static_cast<double>(e.get_raw_phase()); });
            ImPlot::PlotLine("RAW_PHASE [rad]", frequency_vector.data(), raw_phase_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
    }

    void calibration_data_plots() {
        const auto start_freq = measure_captures.load()->config.get_start_freq();
        const auto inc_freq = measure_captures.load()->config.get_inc_freq();
        std::vector<double> frequency_vector(calibration_data.size());
        std::generate(frequency_vector.begin(), frequency_vector.end(), [start_freq, inc_freq, n = 0.0] () mutable { return static_cast<double>(start_freq + ((n++) * inc_freq)); });

        if(ImPlot::BeginPlot("Calibration Gain Factor")) {
            ImPlot::SetupAxes("f [Hz]", "GAIN_FACTOR");
            std::vector<double> gain_factor_vector(calibration_data.size());
            const auto calibration_impedance = measure_captures.load()->config.calibration_impedance;
            std::transform(calibration_data.begin(), calibration_data.end(), gain_factor_vector.begin(), [calibration_impedance](AD5933_CalibrationData &e) { return static_cast<double>(e.get_gain_factor(calibration_impedance)); });
            ImPlot::PlotLine("GAIN_FACTOR", frequency_vector.data(), gain_factor_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
         if(ImPlot::BeginPlot("Calibration System Phase")) {
            ImPlot::SetupAxes("f [Hz]", "SYSTEM_PHASE");
            std::vector<double> raw_phase_vector(calibration_data.size());
            std::transform(calibration_data.begin(), calibration_data.end(), raw_phase_vector.begin(), [](AD5933_CalibrationData &e) { return static_cast<double>(e.get_system_phase()); });
            ImPlot::PlotLine("SYSTEM_PHASE [rad]", frequency_vector.data(), raw_phase_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
    }

    void create_plot_calibration_window(bool &running) {
        ImPlot::CreateContext();
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(600, 750), ImGuiCond_FirstUseEver);
        ImGui::Begin("Calibration Plots", &running);

        if(ImGui::BeginTabBar("Calibration_PlotsBar")) {
            if (ImGui::BeginTabItem("RAW_DATA")) {
                calibration_raw_data_plots();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("CALCULATED_DATA")) {
                calibration_calculated_data_plots();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("CALIBRATION_DATA")) {
                calibration_data_plots();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();

        ImGui::End();
    }
}

namespace ImGuiSDL {
    void freq_sweep_raw_data_plots() {
        const auto start_freq = measure_captures.load()->config.get_start_freq();
        const auto inc_freq = measure_captures.load()->config.get_inc_freq();
        std::vector<double> frequency_vector(calibration_data.size());
        std::generate(frequency_vector.begin(), frequency_vector.end(), [start_freq, inc_freq, n = 0.0] () mutable { return static_cast<double>(start_freq + ((n++) * inc_freq)); });

        if(ImPlot::BeginPlot("Measurement Raw Real Data")) {
            ImPlot::SetupAxes("f [Hz]", "REAL_DATA");
            std::vector<double> real_data_vector(measurement_data.size());
            std::transform(measurement_data.begin(), measurement_data.end(), real_data_vector.begin(), [](AD5933_MeasurementData &e) { return static_cast<double>(e.get_real_part()); });
            ImPlot::PlotLine("REAL_DATA [1/Ohm]", frequency_vector.data(), real_data_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
         if(ImPlot::BeginPlot("Measurement Raw Imag Data")) {
            ImPlot::SetupAxes("f [Hz]", "IMAG_DATA");
            std::vector<double> imag_data_vector(measurement_data.size());
            std::transform(measurement_data.begin(), measurement_data.end(), imag_data_vector.begin(), [](AD5933_MeasurementData &e) { return static_cast<double>(e.get_imag_part()); });
            ImPlot::PlotLine("IMAG_DATA [1/Ohm]", frequency_vector.data(), imag_data_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
    }

    void freq_sweep_calculated_data_plots() {
        const auto start_freq = measure_captures.load()->config.get_start_freq();
        const auto inc_freq = measure_captures.load()->config.get_inc_freq();
        std::vector<double> frequency_vector(calibration_data.size());
        std::generate(frequency_vector.begin(), frequency_vector.end(), [start_freq, inc_freq, n = 0.0] () mutable { return static_cast<double>(start_freq + ((n++) * inc_freq)); });

        if(ImPlot::BeginPlot("Measurement Calculated Magnitude")) {
            ImPlot::SetupAxes("f [Hz]", "RAW_MAGNITUDE");
            std::vector<double> raw_magnitude_vector(measurement_data.size());
            std::transform(measurement_data.begin(), measurement_data.end(), raw_magnitude_vector.begin(), [](AD5933_MeasurementData &e) { return static_cast<double>(e.get_raw_magnitude()); });
            ImPlot::PlotLine("RAW_MAGNITUDE [1/Ohm]", frequency_vector.data(), raw_magnitude_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
         if(ImPlot::BeginPlot("Measurement Calculated Phase")) {
            ImPlot::SetupAxes("f [Hz]", "RAW_PHASE");
            std::vector<double> raw_phase_vector(measurement_data.size());
            std::transform(measurement_data.begin(), measurement_data.end(), raw_phase_vector.begin(), [](AD5933_MeasurementData &e) { return static_cast<double>(e.get_raw_phase()); });
            ImPlot::PlotLine("RAW_PHASE [rad]", frequency_vector.data(), raw_phase_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
    }

    void freq_sweep_corrected_data_plots() {
        const auto start_freq = measure_captures.load()->config.get_start_freq();
        const auto inc_freq = measure_captures.load()->config.get_inc_freq();
        std::vector<double> frequency_vector(calibration_data.size());
        std::generate(frequency_vector.begin(), frequency_vector.end(), [start_freq, inc_freq, n = 0.0] () mutable { return static_cast<double>(start_freq + ((n++) * inc_freq)); });

        if(ImPlot::BeginPlot("Measurement Corrected Data")) {
            ImPlot::SetupAxes("f [Hz]", "CORRECTED_IMPEDANCE");
            std::vector<double> corrected_impedance_vector(measurement_data.size());
            const auto calibration_impedance = measure_captures.load()->config.calibration_impedance;
            std::transform(measurement_data.begin(), measurement_data.end(), corrected_impedance_vector.begin(), [cur_index = 0, calibration_impedance](AD5933_MeasurementData &e) mutable {
                return e.get_corrected_magnitude(calibration_data[cur_index++].get_gain_factor(calibration_impedance));
            });
            ImPlot::PlotLine("CORRECTED_IMPEDANCE [Ohm]", frequency_vector.data(), corrected_impedance_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
         if(ImPlot::BeginPlot("Measurement Calculated Phase")) {
            ImPlot::SetupAxes("f [Hz]", "CORRECTED_PHASE");
            std::vector<double> corrected_phase_vector(measurement_data.size());
            std::transform(measurement_data.begin(), measurement_data.end(), corrected_phase_vector.begin(), [cur_index = 0](AD5933_MeasurementData &e) mutable {
                return static_cast<double>(e.get_corrected_phase(calibration_data[cur_index++].get_system_phase()));
            });
            ImPlot::PlotLine("CORRECTED_PHASE [rad]", frequency_vector.data(), corrected_phase_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
    }

    void freq_sweep_impedance_data_plots() {
        const auto start_freq = measure_captures.load()->config.get_start_freq();
        const auto inc_freq = measure_captures.load()->config.get_inc_freq();
        std::vector<double> frequency_vector(calibration_data.size());
        std::generate(frequency_vector.begin(), frequency_vector.end(), [start_freq, inc_freq, n = 0.0] () mutable { return static_cast<double>(start_freq + ((n++) * inc_freq)); });
        const auto calibration_impedance = measure_captures.load()->config.calibration_impedance;

        if(ImPlot::BeginPlot("Measurement Resistance Data")) {
            ImPlot::SetupAxes("f [Hz]", "RESISTANCE");
            std::vector<double> resistance_vector(measurement_data.size());
            std::transform(measurement_data.begin(), measurement_data.end(), resistance_vector.begin(), [cur_index = 0, calibration_impedance](AD5933_MeasurementData &e) mutable {
                const auto ret = e.get_corrected_resistance(
                    calibration_data[cur_index].get_gain_factor(calibration_impedance),
                    calibration_data[cur_index].get_system_phase()
                );
                cur_index++;
                return ret;
            });
            ImPlot::PlotLine("RESISTANCE [Ohm]", frequency_vector.data(), resistance_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }
        if(ImPlot::BeginPlot("Measurement Reactance Data")) {
            ImPlot::SetupAxes("f [Hz]", "REACTANCE");
            std::vector<double> reactance_vector(measurement_data.size());
            std::transform(measurement_data.begin(), measurement_data.end(), reactance_vector.begin(), [cur_index = 0, calibration_impedance](AD5933_MeasurementData &e) mutable {
                const auto ret = e.get_corrected_reactance(
                    calibration_data[cur_index].get_gain_factor(calibration_impedance),
                    calibration_data[cur_index].get_system_phase()
                );
                cur_index++;
                return ret;
            });
            ImPlot::PlotLine("REACTANCE [Ohm]", frequency_vector.data(), reactance_vector.data(), frequency_vector.size());
            ImPlot::EndPlot();
        }

    }

    void create_freq_sweep_window_enable(bool &running) {
        ImPlot::CreateContext();
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(600, 750), ImGuiCond_FirstUseEver);
        ImGui::Begin("Frequency Sweep Plots", &running);

        if(ImGui::BeginTabBar("FREQ_SWEEP_BAR")) {
            if (ImGui::BeginTabItem("RAW_DATA")) {
                freq_sweep_raw_data_plots();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("CALCULATED_DATA")) {
                freq_sweep_calculated_data_plots();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("CORRECTED_DATA")) {
                freq_sweep_corrected_data_plots();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("IMPEDANCE_DATA")) {
                freq_sweep_impedance_data_plots();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();

        ImGui::End();
    }
}

namespace ImGuiSDL {
    inline void create_connecting_window() {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        connecting_window();
    }

    inline void create_main_window(std::optional<ESP32_AD5933> &esp32_ad5933) {
        static ImVec2 measurement_window_size = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(measurement_window_size);
        static bool plot_calibration_window_enable = false;
        static bool plot_freq_sweep_window_enable = false;
        measurement_window(measurement_window_size, esp32_ad5933, plot_calibration_window_enable, plot_freq_sweep_window_enable);
        if(debug_window_enable.load()) {
            ImGui::SetNextWindowPos(ImVec2(measurement_window_size.x, 0.0f));
            ImGui::SetNextWindowSize(measurement_window_size);
            debug_registers_window(esp32_ad5933);
        }
        if(plot_calibration_window_enable) {
            create_plot_calibration_window(plot_calibration_window_enable);
        }
        if(plot_freq_sweep_window_enable) {
            create_freq_sweep_window_enable(plot_freq_sweep_window_enable);
        }
    }
}

namespace ImGuiSDL {
    void loop(std::optional<ESP32_AD5933> &esp32_ad5933) {
        SDL_Window* window;
        SDL_Renderer* renderer;
        {
            const auto ret = init();
            window = std::get<0>(ret);
            renderer = std::get<1>(ret);
        }

        bool done = false;
        const ImVec4 clear_color { 0.45f, 0.55f, 0.60f, 1.00f };

        while(
            (esp32_ad5933.has_value() == false || esp32_ad5933.value().is_connected() == false)
            && done == false
        ) {
            process_events(window, done);
            start_new_frame();
            create_connecting_window();
            render(renderer, clear_color);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        while(done == false) {
            process_events(window, done);
            start_new_frame();
            create_main_window(esp32_ad5933);
            render(renderer, clear_color);
        }

        shutdown(renderer, window);
    }
}
