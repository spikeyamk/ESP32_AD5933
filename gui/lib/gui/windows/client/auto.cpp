#include <iostream>
#include <thread>

#include "imgui_internal.h"
#include <utf/utf.hpp>

#include "misc/variant_tester.hpp"
#include "magic/misc/gettimeofday.hpp"

#include "gui/windows/client/auto.hpp"

namespace GUI {
    namespace Windows {
        Auto::Auto(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) :
            index { index },
            shm { shm }
        {
            name.append(utf::as_u8(std::to_string(index)));
        }

        Auto::~Auto() {
            stop_source.request_stop();
        }

        const std::optional<Lock> Auto::draw_inner() {
            if(status == Status::Off) {
                if(ImGui::Button("Start Sending")) {
                    start_sending();
                }
                ImGui::SameLine();
                if(ImGui::Button("Start Saving")) {
                    start_saving();
                }
                ImGui::BeginDisabled();
                ImGui::SameLine();
                ImGui::Button("Stop");
                ImGui::EndDisabled();
            } else if(status == Status::Sending || status == Status::Saving) {
                ImGui::BeginDisabled();
                ImGui::Button("Start Sending");
                ImGui::SameLine();
                ImGui::Button("Start Saving");
                ImGui::EndDisabled();
                ImGui::SameLine();
                if(ImGui::Button("Stop")) {
                    stop();
                }

                return Lock::Auto;
            }

            return std::nullopt;
        }

        void Auto::draw(bool &enable, const ImGuiID side_id, Lock& lock) {
            if(first) {
                ImGui::DockBuilderDockWindow((const char*) name.c_str(), side_id);
                first = false;
            }

            if(ImGui::Begin((const char*) name.c_str(), &enable) == false) {
                ImGui::End();
                return;
            }

            if(lock != Lock::Released && lock != Lock::Auto) {
                ImGui::BeginDisabled();
                draw_inner();
                ImGui::EndDisabled();
            } else {
                const auto ret_lock { draw_inner() };
                if(lock == Lock::Auto && ret_lock.has_value() == false) {
                    lock = Lock::Released;
                } else if(ret_lock.has_value()) {
                    lock = ret_lock.value();
                }
            }
            ImGui::End();
        }

        void Auto::start_saving() {
            status = Status::Saving;
        }

        void Auto::start_sending() {
            std::jthread t1(sending_cb, std::ref(*this));
            stop_source = t1.get_stop_source();
            t1.detach();
        }

        void Auto::stop() {
            stop_source.request_stop();
        }

        void Auto::sending_cb(std::stop_token st, Auto& self) {
            self.status = Status::Sending;
            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Events::Commands::Auto::Send{}
                }
            );
            while(st.stop_requested() == false) {
                const auto rx_payload { self.shm->active_devices[self.index].measurement->read_for(boost::posix_time::milliseconds(10'000)) };
                if(rx_payload.has_value() == false) {
                    std::cout << "ERROR: GUI::Windows::Auto::sending_cb: rx_payload: timeout\n";
                    self.status = Status::Error;
                    return;
                }

                if(variant_tester<Magic::Events::Results::Auto::Point>(rx_payload.value()) == false) {
                    std::cout << "ERROR: GUI::Windows::Auto::sending_cb: rx_payload: bad variant type\n";
                    self.status = Status::Error;
                    return;
                }

                std::visit([&self](auto&& event) {
                    if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Events::Results::Auto::Point>) {
                        mytimeval64_t tv;
                        gettimeofday(&tv, nullptr);
                        const double seconds = static_cast<double>(tv.tv_sec);
                        const double useconds_to_seconds = static_cast<double>(tv.tv_usec) / 1000000.0;
                        const Point point {
                            .time = seconds + useconds_to_seconds,
                            .auto_meas = event,
                        };
                        self.send_points.push(point);
                    }
                }, rx_payload.value());
            }
            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Events::Commands::Auto::End{}
                }
            );
            self.status = Status::Off;
        }
    }
}