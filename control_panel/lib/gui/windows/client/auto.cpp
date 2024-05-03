#include <iostream>
#include <thread>

#include <imgui_internal.h>
#include <utf/utf.hpp>

#include "misc/variant_tester.hpp"
#include "magic/misc/gettimeofday.hpp"
#include "imgui_custom/spinner.hpp"
#include "gui/boilerplate.hpp"

#include "gui/windows/client/auto.hpp"

namespace GUI {
    namespace Windows {
        Auto::Auto(const size_t index, std::shared_ptr<BLE_Client::SHM::SHM> shm) :
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
                ImGui::SliderScalar("Tick [ms]", ImGuiDataType_U16, &tick_ms, &tick_ms_min, &tick_ms_max, "%u");

                if(ImGui::Button("Send", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    start_sending();
                }

                if(ImGui::Button("Save", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    start_saving();
                }

                ImGui::BeginDisabled();
                ImGui::Button("Stop", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f));
                ImGui::EndDisabled();

                ImGui::Separator();
                ImGui::Text("Total: %llu [Bytes]\nUsed: %llu [Bytes]", bytes.total, bytes.used);

                if(ImGui::Button("List", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    list();
                }

                if(ImGui::Button("Format", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    popup_shows = PopupShows::Format;
                }
            } else {
                ImGui::BeginDisabled();

                ImGui::SliderScalar("Tick [ms]", ImGuiDataType_U16, &tick_ms, &tick_ms_min, &tick_ms_max, "%u");

                ImGui::Button("Send", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f));
                if(status == Status::Sending) {
                    ImGui::SameLine();
                    ImGui::EndDisabled();
                    const float scale { GUI::Boilerplate::get_scale() };
                    ImGui::Spinner("SendingSinner", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    ImGui::BeginDisabled();
                }

                ImGui::Button("Save", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f));
                if(status == Status::Saving) {
                    ImGui::SameLine();
                    ImGui::EndDisabled();
                    const float scale { GUI::Boilerplate::get_scale() };
                    ImGui::Spinner("SavingSinner", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    ImGui::BeginDisabled();
                }

                if(status == Status::Saving || status == Status::Sending) {
                    ImGui::EndDisabled();
                    if(ImGui::Button("Stop", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                        stop();
                    }
                    ImGui::BeginDisabled();
                } else {
                    ImGui::Button("Stop", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f));
                }

                ImGui::Separator();
                ImGui::Text("Total: %llu [Bytes]\nUsed: %llu [Bytes]", bytes.total, bytes.used);

                ImGui::Button("List", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f));
                if(status == Status::Listing) {
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    ImGui::ProgressBar(progress_bar_fraction);
                    ImGui::BeginDisabled();
                }

                ImGui::Button("Format", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f));
                if(status == Status::Formatting) {
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    const float scale { GUI::Boilerplate::get_scale() };
                    ImGui::Spinner("FormattingSpinner", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    ImGui::BeginDisabled();
                }

                ImGui::EndDisabled();
            }

            if(selected.has_value() && status == Status::Off) {
                if(ImGui::Button("Remove", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    popup_shows = PopupShows::Remove;
                }

                if(ImGui::Button("Download", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    download();
                }
            } else {
                ImGui::BeginDisabled();

                ImGui::Button("Remove", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f));
                if(status == Status::Removing) {
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    const float scale { GUI::Boilerplate::get_scale() };
                    ImGui::Spinner("RemovingSpinner", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    ImGui::BeginDisabled();
                }

                ImGui::Button("Download", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f));
                if(status == Status::Downloading) {
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    ImGui::ProgressBar(progress_bar_fraction);
                    ImGui::BeginDisabled();
                }

                ImGui::EndDisabled();
            }

            if(status != Status::Off) {
                ImGui::BeginDisabled();
                draw_list_table();
                ImGui::EndDisabled();
            } else {
                draw_list_table();
            }

            if(status != Status::Off) {
                return Lock::Auto;
            } else {
                return std::nullopt;
            }
        }

        void Auto::draw(bool &enable, const ImGuiID side_id, Lock& lock) {
            if(first) {
                ImGui::DockBuilderDockWindow((const char*) name.c_str(), side_id);
                first = false;
            }

            if(ImGui::Begin((const char*) name.c_str(), &enable, ImGuiWindowFlags_NoMove) == false) {
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
            draw_popups();
        }

        void Auto::draw_popups() {
            switch(popup_shows) {
                case PopupShows::Format:
                    ImGui::OpenPopup(std::string("Format##").append(std::to_string(index)).c_str());
                    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                    popup_shows = PopupShows::Off;
                    break;
                case PopupShows::Remove:
                    ImGui::OpenPopup(std::string("Remove##").append(std::to_string(index)).c_str());
                    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                    popup_shows = PopupShows::Off;
                    break;
                default:
                    break;
            }

            if(ImGui::BeginPopupModal(std::string("Format##").append(std::to_string(index)).c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Are you sure you want to proceed?");
                if(ImGui::Button("OK", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    ImGui::CloseCurrentPopup();
                    format();
                }
                ImGui::SameLine();
                if(ImGui::Button("Cancel", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if(ImGui::BeginPopupModal(std::string("Remove##").append(std::to_string(index)).c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Are you sure you want to proceed?");
                if(ImGui::Button("OK", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    ImGui::CloseCurrentPopup();
                    remove();
                }
                ImGui::SameLine();
                if(ImGui::Button("Cancel", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            
            if(popup.has_value()) {
                if(popup.value().shown == false) {
                    ImGui::OpenPopup(popup.value().name.c_str());
                    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                    popup.value().shown = true;
                }

                if(ImGui::BeginPopupModal(popup.value().name.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text(popup.value().content.c_str());
                    if(ImGui::Button("OK", ImVec2(64.0f * GUI::Boilerplate::get_scale(), 0.0f))) {
                        ImGui::CloseCurrentPopup();
                        popup = std::nullopt;
                    }
                    ImGui::EndPopup();
                }
            }
        }
    }

    namespace Windows {
        void Auto::draw_list_table() {
            if(ImGui::BeginTable("List Table", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Path");
                ImGui::TableNextColumn();
                ImGui::Text("Size [Bytes]");
                if(
                    list_table.paths.size() != 0
                    && list_table.sizes.size() != 0
                    && list_table.paths.size() == list_table.sizes.size()
                ) {
                    draw_list_table_rows();
                } else {
                    ImGui::BeginDisabled();
                    draw_list_table_rows();
                    ImGui::EndDisabled();
                }
            }
            ImGui::EndTable();
        }

        void Auto::draw_list_table_rows() {
            for(size_t i = 0; i < list_table.paths.size() && i < list_table.sizes.size(); i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::PushID(static_cast<int>(i));
                if(ImGui::Selectable(std::string(list_table.paths[i].path.begin(), list_table.paths[i].path.end()).c_str(), i == selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    if(selected == i) {
                        selected = std::nullopt;
                    } else {
                        selected = i;
                    }
                }
                ImGui::PopID();
                ImGui::TableNextColumn();
                ImGui::PushID(-static_cast<int>(i));
                ImGui::Text(std::to_string(list_table.sizes[i].num_of_bytes).c_str());
                ImGui::PopID();
            }
        }

        void Auto::start_saving() {
            status = Status::Saving;
            shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    index,
                    Magic::Commands::Auto::Save{ .tick_ms = tick_ms }
                }
            );
        }

        void Auto::start_sending() {
            std::jthread t1(start_sending_cb, std::ref(*this));
            stop_source = t1.get_stop_source();
            t1.detach();
        }

        void Auto::stop() {
            shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    index,
                    Magic::Commands::Auto::End{}
                }
            );

            if(status == Status::Sending) {
                stop_source.request_stop();
            }

            status = Status::Off;
        }

        void Auto::start_sending_cb(std::stop_token st, Auto& self) {
            self.status = Status::Sending;
            self.shm->active_devices[self.index].measurement->clear();
            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Commands::Auto::Send{ .tick_ms = self.tick_ms }
                }
            );
            while(st.stop_requested() == false) {
                const auto rx_payload { self.shm->active_devices[self.index].measurement->read_for(std::chrono::milliseconds(10'000)) };
                if(rx_payload.has_value() == false) {
                    std::cout << "ERROR: GUI::Windows::Auto::start_sending_cb: rx_payload: timeout\n";
                    self.status = Status::Error;
                    const std::optional<Popup> tmp_popup {
                        {
                            std::string("Timeout##").append(std::to_string(self.index)),
                            "GUI::Windows::Auto::start_sending_cb: rx_payload: timeout"
                        }
                    };
                    self.popup = tmp_popup;
                    return;
                }

                if(variant_tester<Magic::Results::Auto::Point>(rx_payload.value()) == false) {
                    std::cout << "ERROR: GUI::Windows::Auto::start_sending_cb: rx_payload: bad variant type\n";
                    self.status = Status::Error;
                    const std::optional<Popup> tmp_popup {
                        {
                            std::string("Bad variant type##").append(std::to_string(self.index)),
                            "GUI::Windows::Auto::start_sending_cb: rx_payload: bad variant type"
                        }
                    };
                    self.popup = tmp_popup;
                    return;
                }

                std::visit([&self](auto&& event) {
                    if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Results::Auto::Point>) {
                        const auto tmp_timeval { Magic::Commands::Time::UpdateTimeval::now() };
                        const double useconds_to_seconds = static_cast<double>(tmp_timeval.tv_sec) + (static_cast<double>(tmp_timeval.tv_usec) / 1000000.0);
                        const Point point {
                            .time = useconds_to_seconds,
                            .auto_meas = event,
                        };
                        self.send_points->push(point);
                    }
                }, rx_payload.value());
            }
            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Commands::Auto::End{}
                }
            );
            self.status = Status::Off;
        }
    }

    namespace Windows {
        void Auto::list() {
            selected = std::nullopt;
            progress_bar_fraction = 0.0f;
            std::jthread t1(list_cb, std::ref(*this));
            t1.detach();
            stop_source = t1.get_stop_source();
        }

        void Auto::remove() {
            status = Status::Removing;
            std::thread(remove_cb, std::ref(*this)).detach();
        }

        void Auto::remove_cb(Auto& self) {
            self.shm->active_devices[self.index].information->clear();
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::Start{} });
            const Magic::Commands::File::Remove remove_event { .path = self.list_table.paths[self.selected.value()].path };
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, remove_event });
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::End{} });

            const auto remove_payload { self.shm->active_devices[self.index].information->read_for(std::chrono::milliseconds(5'000)) };

            if(remove_payload.has_value() == false) {
                std::cout << "GUI::Windows::Auto::remove_cb: remove_payload timeout\n";
                self.status = Status::Error;
                const std::optional<Popup> tmp_popup {
                    {
                        std::string("Timeout##").append(std::to_string(self.index)),
                        "GUI::Windows::Auto::remove_cb: remove_payload timeout"
                    }
                };
                self.popup = tmp_popup;
                return;
            }

            if(variant_tester<Magic::Results::File::Remove>(remove_payload.value()) == false) {
                std::cout << "GUI::Windows::Auto::remove_cb: remove_payload wrong variant type\n";
                self.status = Status::Error;
                return;
            }

            if(
                [&remove_payload]() {
                    bool ret = false;
                    std::visit([&remove_payload, &ret](auto&& event) {
                        if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Results::File::Remove>) {
                            ret = event.status;
                        }
                    }, remove_payload.value());
                    return ret;
                }() == false
            ) {
                std::cout << "GUI::Windows::Auto::remove_cb: remove_payload.status == false\n";
                self.status = Status::Error;
                const std::optional<Popup> tmp_popup {
                    {
                        std::string("Remove##").append(std::to_string(self.index)),
                        "GUI::Windows::Auto::remove_cb: remove_payload.status == false"
                    }
                };
                self.popup = tmp_popup;
                return;
            }

            self.list();
        }

        void Auto::download() {
            progress_bar_fraction = 0.0f;
            std::jthread t1(download_cb, std::ref(*this));
            t1.detach();
            stop_source = t1.get_stop_source();
        }

        void Auto::format() {
            selected = std::nullopt;
            std::jthread t1(format_cb, std::ref(*this));
            t1.detach();
            stop_source = t1.get_stop_source();
        }

        void Auto::list_cb(std::stop_token st, Auto& self) {
            self.status = Status::Listing;
            self.shm->active_devices[self.index].information->clear();
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::Start{} });

            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::Free{} });
            const auto free_payload { self.shm->active_devices[self.index].information->read_for(std::chrono::milliseconds(5'000)) };
            if(free_payload.has_value() == false) {
                std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive free: timeout\n";
                self.status = Status::Error;
                const std::optional<Popup> tmp_popup {
                    {
                        std::string("Timeout##").append(std::to_string(self.index)),
                        "GUI::Windows::RecordManager::list_cb: failed to retreive free: timeout"
                    }
                };
                self.popup = tmp_popup;
                return;
            }

            if(variant_tester<Magic::Results::File::Free>(free_payload.value()) == false) {
                std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive free: wrong variant type\n";
                self.status = Status::Error;
                const std::optional<Popup> tmp_popup {
                    {
                        std::string("Bad variant type##").append(std::to_string(self.index)),
                        "GUI::Windows::RecordManager::list_cb: failed to retreive free: wrong variant type"
                    }
                };
                self.popup = tmp_popup;
                return;
            }

            std::visit([&self](auto&& event) {
                if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Results::File::Free>) {
                    self.bytes.used = event.used_bytes;
                    self.bytes.total = event.total_bytes;
                }
            }, free_payload.value());

            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::ListCount{} });
            const auto list_count_payload { self.shm->active_devices[self.index].information->read_for(std::chrono::milliseconds(5'000)) };

            if(list_count_payload.has_value() == false) { 
                std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_count: timeout\n";
                self.status = Status::Error;
                const std::optional<Popup> tmp_popup {
                    {
                        std::string("Timeout##").append(std::to_string(self.index)),
                        "GUI::Windows::RecordManager::list_cb: failed to retreive list_count: timeout"
                    }
                };
                self.popup = tmp_popup;
                return;
            }
            
            if(variant_tester<Magic::Results::File::ListCount>(list_count_payload.value()) == false) {
                self.status = Status::Error;
                std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_count: bad variant type\n";
                const std::optional<Popup> tmp_popup {
                    {
                        std::string("Bad variant type##").append(std::to_string(self.index)),
                        "GUI::Windows::RecordManager::list_cb: failed to retreive list_count: bad variant type"
                    }
                };
                self.popup = tmp_popup;
                return;
            }

            Magic::Results::File::ListCount list_count { .num_of_files = 0 };
            std::visit([&list_count](auto&& event) {
                if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Results::File::ListCount>) {
                    list_count = event;
                }
            }, list_count_payload.value());

            if(list_count.num_of_files == 0) {
                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::End{} });
                std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_count: list_cout empty\n";
                const std::optional<Popup> tmp_popup {
                    {
                        std::string("List##").append(std::to_string(self.index)),
                        "GUI::Windows::RecordManager::list_cb: failed to retreive list_count: list_cout empty"
                    }
                };
                self.popup = tmp_popup;
                self.status = Status::Off;
            }

            const float progress_bar_step { (1.0f / static_cast<float>(list_count.num_of_files)) / 2.0f };

            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::List{} });
            std::vector<Magic::Results::File::List> listed_paths;
            listed_paths.reserve(list_count.num_of_files);
            while(listed_paths.size() != list_count.num_of_files) {
                const auto list_payload { self.shm->active_devices[self.index].information->read_for(std::chrono::milliseconds(5'000)) };
                if(list_payload.has_value() == false) {
                    std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_path: timeout\n";
                    self.status = Status::Error;
                    const std::optional<Popup> tmp_popup {
                        {
                            std::string("Timeout##").append(std::to_string(self.index)),
                            "GUI::Windows::RecordManager::list_cb: failed to retreive list_path: timeout"
                        }
                    };
                    self.popup = tmp_popup;
                    return;
                }

                if(variant_tester<Magic::Results::File::List>(list_payload.value()) == false) {
                    std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_path: bad variant type\n";
                    self.status = Status::Error;
                    const std::optional<Popup> tmp_popup {
                        {
                            std::string("Bad variant type##").append(std::to_string(self.index)),
                            "GUI::Windows::RecordManager::list_cb: failed to retreive list_path: bad variant type"
                        }
                    };
                    self.popup = tmp_popup;
                    return;
                }

                std::visit([&listed_paths](auto&& event) {
                    if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Results::File::List>) {
                       listed_paths.push_back(event);
                    }
                }, list_payload.value());

                self.progress_bar_fraction += progress_bar_step;
            }

            std::vector<Magic::Results::File::Size> listed_sizes;
            listed_sizes.reserve(list_count.num_of_files);
            for(const auto& path: listed_paths) {
                const Magic::Commands::File::Size size_event { .path = path.path };
                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, size_event });
                const auto size_payload { self.shm->active_devices[self.index].information->read_for(std::chrono::milliseconds(5'000)) };

                if(size_payload.has_value() == false) {
                    std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_size: timeout\n";
                    self.status = Status::Error;
                    const std::optional<Popup> tmp_popup {
                        {
                            std::string("Timeout##").append(std::to_string(self.index)),
                            "GUI::Windows::RecordManager::list_cb: failed to retreive list_size: timeout"
                        }
                    };
                    self.popup = tmp_popup;
                    return;
                }

                if(variant_tester<Magic::Results::File::Size>(size_payload.value()) == false) {
                    std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_size: bad variant type\n";
                    self.status = Status::Error;
                    const std::optional<Popup> tmp_popup {
                        {
                            std::string("Bad variant type##").append(std::to_string(self.index)),
                            "GUI::Windows::RecordManager::list_cb: failed to retreive list_size: bad variant type"
                        }
                    };
                    self.popup = tmp_popup;
                    return;
                }

                std::visit([&listed_sizes](auto&& event) {
                    if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Results::File::Size>) {
                        listed_sizes.push_back(event);
                    }
                }, size_payload.value());

                self.progress_bar_fraction += progress_bar_step;
            }

            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::End{} });

            self.list_table.paths = listed_paths;
            self.list_table.sizes = listed_sizes;

            self.status = Status::Off;
        }

        void Auto::download_cb(std::stop_token st, Auto& self) {
            try {
                self.status = Status::Downloading;
                self.shm->active_devices[self.index].information->clear();
                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::Start{} });
                const Magic::Commands::File::Download download_event { .path = self.list_table.paths[self.selected.value()].path };
                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, download_event });
                std::vector<Magic::Results::File::Download> download_slices;
                const size_t wished_slices_size = static_cast<size_t>(std::ceil(static_cast<float>(self.list_table.sizes[self.selected.value()].num_of_bytes) / static_cast<float>(sizeof(Magic::T_MaxDataSlice))));
                const float progress_bar_step = 1.0f / static_cast<float>(wished_slices_size);
                const uint64_t wished_file_size = self.list_table.sizes[self.selected.value()].num_of_bytes;
                download_slices.reserve(wished_slices_size);
                while(download_slices.size() < wished_slices_size) {
                    if(st.stop_requested()) {
                        std::cout << "ERROR: GUI::Windows::RecordManager::download_cb: failed to retreive download_payload: stop_requested\n";
                        self.status = Status::Error;
                        return;
                    }

                    const auto download_payload { self.shm->active_devices[self.index].information->read_for(std::chrono::milliseconds(64'000)) };
                    if(download_payload.has_value() == false) {
                        std::cout << "ERROR: GUI::Windows::RecordManager::download_cb: failed to retreive download_payload: timeout\n";
                        self.status = Status::Error;
                        const std::optional<Popup> tmp_popup {
                            {
                                std::string("Timeout##").append(std::to_string(self.index)),
                                "GUI::Windows::RecordManager::download_cb: failed to retreive download_payload: timeout"
                            }
                        };
                        self.popup = tmp_popup;
                        return;
                    }

                    if(variant_tester<Magic::Results::File::Download>(download_payload.value()) == false) {
                        std::cout << "ERROR: GUI::Windows::RecordManager::download_cb: failed to retreive download_payload: wrong variant type\n";
                        self.status = Status::Error;
                        const std::optional<Popup> tmp_popup {
                            {
                                std::string("Bad variant type##").append(std::to_string(self.index)),
                                "GUI::Windows::RecordManager::download_cb: failed to retreive download_payload: bad variant type"
                            }
                        };
                        self.popup = tmp_popup;
                        return;
                    }

                    std::visit([&download_slices](auto&& event) {
                        if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Results::File::Download>) {
                            download_slices.push_back(event);
                        }
                    }, download_payload.value());
                    self.progress_bar_fraction += progress_bar_step;
                }
                
                {
                    uint64_t i = 0;
                    std::vector<uint8_t> file_content;
                    file_content.reserve(wished_file_size);
                    for(const auto& slice: download_slices) {
                        for(const uint8_t e: slice.slice) {
                            if(i == wished_file_size) {
                                break;
                            }
                            file_content.push_back(e);
                            i++;
                        }
                    }

                    for(
                        auto it = file_content.begin();
                        it < file_content.end() && it + sizeof(Magic::Results::Auto::Record::Entry) < file_content.end();
                        it += sizeof(Magic::Results::Auto::Record::Entry)
                    ) {
                        std::array<uint8_t, sizeof(Magic::Results::Auto::Record::Entry)> entry_serialized { 0 };
                        std::copy(it, it + sizeof(Magic::Results::Auto::Record::Entry), entry_serialized.begin());

                        const auto entry { Magic::Results::Auto::Record::Entry::from_raw_data(entry_serialized) };

                        self.save_points->push(
                            {
                                .time = entry.unix_timestamp,
                                .auto_meas = {
                                    .impedance = entry.impedance,
                                    .phase = entry.phase,
                                }
                            } 
                        );
                    }
                }

                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::End{} });
                self.status = Status::Off;
           } catch(const std::exception& e) {
                std::cout << "ERROR: GUI::Windows::RecordManager::download_cb: exception: " << e.what() << std::endl;
                self.status = Status::Error;
                const std::optional<Popup> tmp_popup {
                    {
                        std::string("Exception##").append(std::to_string(self.index)),
                        std::string("GUI::Windows::RecordManager::download_cb: exception: ").append(e.what())
                    }
                };
                self.popup = tmp_popup;
            }
        }

        void Auto::format_cb(std::stop_token st, Auto& self) {
            self.status = Status::Formatting;
            self.shm->active_devices[self.index].information->clear();
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::Start{} });
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::Format{} });
            const auto format_payload { self.shm->active_devices[self.index].information->read_for(std::chrono::milliseconds(64'000)) };

            if(format_payload.has_value() == false) {
                std::cout << "GUI::Windows::RecordManager::format_cb: timeout\n";
                self.status = Status::Error;
                const std::optional<Popup> tmp_popup {
                    {
                        std::string("Timeout##").append(std::to_string(self.index)),
                        "GUI::Windows::RecordManager::format_cb: timeout"
                    }
                };
                self.popup = tmp_popup;
                return;
            }

            if(variant_tester<Magic::Results::File::Format>(format_payload.value()) == false) {
                std::cout << "GUI::Windows::RecordManager::format_cb: format_payload: bad variant type\n";
                self.status = Status::Error;
                const std::optional<Popup> tmp_popup {
                    {
                        std::string("Bad variant type##").append(std::to_string(self.index)),
                        "GUI::Windows::RecordManager::format_cb: format_payload: bad variant type"
                    }
                };
                self.popup = tmp_popup;
                return;
            }

            if(
                [&]() {
                    bool ret { false };
                    std::visit([&](auto&& event) {
                        if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Results::File::Format>) {
                            ret = event.status;
                        }
                    }, format_payload.value());
                    return ret;
                }() == false
            ) {
                std::cout << "GUI::Windows::RecordManager::format_cb: format_payload.value().status == false\n";
                self.status = Status::Error;
                const std::optional<Popup> tmp_popup {
                    {
                        std::string("Format##").append(std::to_string(self.index)),
                        "GUI::Windows::RecordManager::format_cb: format_payload.value().status == false"
                    }
                };
                self.popup = tmp_popup;
                return;
            }

            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Commands::File::End{} });
            self.status = Status::Off;
            self.list();
        }
    }
}