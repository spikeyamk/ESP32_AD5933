#include <iostream>
#include <thread>

#include "imgui_internal.h"
#include <utf/utf.hpp>
#include <nfd.hpp>

#include "misc/variant_tester.hpp"
#include "magic/misc/gettimeofday.hpp"
#include "imgui_custom/spinner.hpp"
#include "gui/boilerplate.hpp"

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
                ImGui::SliderScalar("Tick [ms]", ImGuiDataType_U16, &tick_ms, &tick_ms_min, &tick_ms_max, "%u");

                if(ImGui::Button("Start Sending")) {
                    start_sending();
                }

                if(ImGui::Button("Start Saving")) {
                    start_saving();
                }

                ImGui::BeginDisabled();
                ImGui::Button("Stop");
                ImGui::EndDisabled();

                if(ImGui::Button("List")) {
                    list();
                }

                if(ImGui::Button("Format")) {
                    format();
                }

                if(ImGui::Button("Create Test Files")) {
                    create_test_files();
                }
            } else {
                ImGui::BeginDisabled();

                ImGui::SliderScalar("Tick [ms]", ImGuiDataType_U16, &tick_ms, &tick_ms_min, &tick_ms_max, "%u");

                ImGui::Button("Start Sending");
                if(status == Status::Sending) {
                    const float scale { GUI::Boilerplate::get_scale() };
                    ImGui::SameLine();
                    ImGui::EndDisabled();
                    Spinner::Spinner("SendingSinner", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    ImGui::BeginDisabled();
                }

                ImGui::Button("Start Saving");
                if(status == Status::Saving) {
                    const float scale { GUI::Boilerplate::get_scale() };
                    ImGui::SameLine();
                    ImGui::EndDisabled();
                    Spinner::Spinner("SavingSinner", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    ImGui::BeginDisabled();
                }

                if(status == Status::Saving || status == Status::Sending) {
                    ImGui::EndDisabled();
                    if(ImGui::Button("Stop")) {
                        stop();
                    }
                    ImGui::BeginDisabled();
                } else {
                    ImGui::Button("Stop");
                }

                ImGui::Button("List");
                if(status == Status::Listing) {
                    const float scale { GUI::Boilerplate::get_scale() };
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    Spinner::Spinner("ListingSpinner", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    ImGui::BeginDisabled();
                }

                ImGui::Button("Format");
                if(status == Status::Formatting) {
                    const float scale { GUI::Boilerplate::get_scale() };
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    Spinner::Spinner("FormattingSpinner", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    ImGui::BeginDisabled();
                }

                ImGui::Button("Create Test Files");
                if(status == Status::CreatingTestFiles) {
                    const float scale { GUI::Boilerplate::get_scale() };
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    Spinner::Spinner("CreatingSpinner", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
                    ImGui::BeginDisabled();
                }

                ImGui::EndDisabled();
            }

            ImGui::Text("Total: %llu [Bytes]", bytes.total);
            ImGui::Text("Used: %llu [Bytes]", bytes.used);

            draw_list_table();

            if(selected.has_value()) {
                if(status == Status::Off) {
                    if(ImGui::Button("Download")) {
                        download();
                    }
                    if(ImGui::Button("Remove")) {
                        remove();
                    }
                } else {
                    ImGui::BeginDisabled();
                    ImGui::Button("Download");
                    if(status == Status::Downloading) {
                        ImGui::EndDisabled();
                        ImGui::SameLine();
                        ImGui::ProgressBar(progress_bar_fraction);
                        ImGui::BeginDisabled();
                    }
                    ImGui::Button("Remove");
                    ImGui::EndDisabled();
                }
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
                    Magic::Events::Commands::Auto::Save{ .tick_ms = tick_ms }
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
                    Magic::Events::Commands::Auto::End{}
                }
            );

            if(status == Status::Sending) {
                stop_source.request_stop();
            }

            status = Status::Off;
        }

        void Auto::start_sending_cb(std::stop_token st, Auto& self) {
            self.status = Status::Sending;
            self.shm->cmd.send(
                BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{
                    self.index,
                    Magic::Events::Commands::Auto::Send{ .tick_ms = self.tick_ms }
                }
            );
            while(st.stop_requested() == false) {
                const auto rx_payload { self.shm->active_devices[self.index].measurement->read_for(boost::posix_time::milliseconds(10'000)) };
                if(rx_payload.has_value() == false) {
                    std::cout << "ERROR: GUI::Windows::Auto::start_sending_cb: rx_payload: timeout\n";
                    self.status = Status::Error;
                    return;
                }

                if(variant_tester<Magic::Events::Results::Auto::Point>(rx_payload.value()) == false) {
                    std::cout << "ERROR: GUI::Windows::Auto::start_sending_cb: rx_payload: bad variant type\n";
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

    namespace Windows {
        void Auto::list() {
            selected = std::nullopt;
            std::jthread t1(list_cb, std::ref(*this));
            t1.detach();
            stop_source = t1.get_stop_source();
        }

        void Auto::remove() {
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::File::Start{} });
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::File::Remove::from_raw_data(list_table.paths[selected.value()].to_raw_data()) });
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::File::End{} });
            list();
        }

        void Auto::download() {
            nfdchar_t* path = nullptr;
            const std::array<nfdfilteritem_t, 1> filterItem { { "Record", "rcd" } };
            const nfdresult_t result = NFD::SaveDialog(path, filterItem.data(), filterItem.size(), nullptr, std::string(list_table.paths[selected.value()].path.begin(), list_table.paths[selected.value()].path.end()).c_str());
            if(result == NFD_OKAY) {
                progress_bar_fraction = 0.0f;
                std::jthread t1(download_cb, std::ref(*this), std::filesystem::path(path));
                t1.detach();
                stop_source = t1.get_stop_source();
                if(path != nullptr) {
                    NFD::FreePath(path);
                    path = nullptr;
                }
            } else if(result == NFD_CANCEL) {
                std::printf("User pressed cancel!\n");
                if(path != nullptr) {
                    NFD::FreePath(path);
                    path = nullptr;
                }
                return;
            } else {
                std::printf("Error: %s\n", NFD::GetError());
                if(path != nullptr) {
                    NFD::FreePath(path);
                    path = nullptr;
                }
                return;
            }
        }

        void Auto::create_test_files() {
            selected = std::nullopt;
            std::jthread t1(create_test_files_cb, std::ref(*this));
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
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::Start{} });

            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::Free{} });
            const auto free_payload { self.shm->active_devices[self.index].information->read_for(boost::posix_time::milliseconds(5'000)) };
            if(free_payload.has_value() == false) {
                self.status = Status::Error;
                std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive free: timeout\n";
                return;
            }

            if(variant_tester<Magic::Events::Results::File::Free>(free_payload.value()) == false) {
                self.status = Status::Error;
                std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive free: wrong variant type\n";
                return;
            }

            std::visit([&self](auto&& event) {
                if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Events::Results::File::Free>) {
                    self.bytes.used = event.used_bytes;
                    self.bytes.total = event.total_bytes;
                }
            }, free_payload.value());

            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::ListCount{} });
            const auto list_count_payload { self.shm->active_devices[self.index].information->read_for(boost::posix_time::milliseconds(5'000)) };

            if(list_count_payload.has_value() == false) { 
                self.status = Status::Error;
                std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_count: timeout\n";
                return;
            }
            
            if(variant_tester<Magic::Events::Results::File::ListCount>(list_count_payload.value()) == false) {
                self.status = Status::Error;
                std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_count: wrong variant type\n";
                return;
            }

            Magic::Events::Results::File::ListCount list_count { .num_of_files = 0 };
            std::visit([&list_count](auto&& event) {
                if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Events::Results::File::ListCount>) {
                    list_count = event;
                }
            }, list_count_payload.value());

            if(list_count.num_of_files == 0) {
                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::End{} });
                std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_count: list_cout empty\n";
                self.status = Status::Off;
            }

            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::List{} });
            std::vector<Magic::Events::Results::File::List> listed_paths;
            listed_paths.reserve(list_count.num_of_files);
            while(listed_paths.size() != list_count.num_of_files) {
                const auto list_payload { self.shm->active_devices[self.index].information->read_for(boost::posix_time::milliseconds(5'000)) };
                if(list_payload.has_value() == false) {
                    std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_path: timeout\n";
                    self.status = Status::Error;
                    return;
                }

                if(variant_tester<Magic::Events::Results::File::List>(list_payload.value()) == false) {
                    std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_path: wrong variant type\n";
                    self.status = Status::Error;
                    return;
                }

                std::visit([&listed_paths](auto&& event) {
                    if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Events::Results::File::List>) {
                       listed_paths.push_back(event);
                    }
                }, list_payload.value());
            }

            std::vector<Magic::Events::Results::File::Size> listed_sizes;
            listed_sizes.reserve(list_count.num_of_files);
            for(const auto& path: listed_paths) {
                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::Size::from_raw_data(path.to_raw_data()) });
                const auto size_payload { self.shm->active_devices[self.index].information->read_for(boost::posix_time::milliseconds(5'000)) };

                if(size_payload.has_value() == false) {
                    std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_size: timeout\n";
                    self.status = Status::Error;
                    return;
                }

                if(variant_tester<Magic::Events::Results::File::Size>(size_payload.value()) == false) {
                    std::cout << "ERROR: GUI::Windows::RecordManager::list_cb: failed to retreive list_size: wrong variant\n";
                    self.status = Status::Error;
                    return;
                }

                std::visit([&listed_sizes](auto&& event) {
                    if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Events::Results::File::Size>) {
                        listed_sizes.push_back(event);
                    }
                }, size_payload.value());
            }

            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::End{} });

            self.list_table.paths = listed_paths;
            self.list_table.sizes = listed_sizes;

            self.status = Status::Off;
        }

        void Auto::download_cb(std::stop_token st, Auto& self, const std::filesystem::path path) {
            try {
                self.status = Status::Downloading;
                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::Start{} });
                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::Download::from_raw_data(self.list_table.paths[self.selected.value()].to_raw_data()) });
                std::vector<Magic::Events::Results::File::Download> download_slices;
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

                    const auto download_payload { self.shm->active_devices[self.index].information->read_for(boost::posix_time::milliseconds(60'000)) };
                    if(download_payload.has_value() == false) {
                        std::cout << "ERROR: GUI::Windows::RecordManager::download_cb: failed to retreive download_payload: timeout\n";
                        self.status = Status::Error;
                        return;
                    }

                    if(variant_tester<Magic::Events::Results::File::Download>(download_payload.value()) == false) {
                        std::cout << "ERROR: GUI::Windows::RecordManager::download_cb: failed to retreive download_payload: wrong variant type\n";
                        self.status = Status::Error;
                        return;
                    }

                    std::visit([&download_slices](auto&& event) {
                        if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Events::Results::File::Download>) {
                            download_slices.push_back(event);
                        }
                    }, download_payload.value());
                    self.progress_bar_fraction += progress_bar_step;
                }
                
                {
                    std::ofstream file(path, std::ios::out | std::ios::binary);
                    for(const auto e: Magic::Events::Results::Auto::Record::file_header) {
                        file << e;
                    }

                    uint64_t i = 0;
                    for(const auto& slice: download_slices) {
                        for(const uint8_t e: slice.slice) {
                            if(i == wished_file_size) {
                                break;
                            }
                            file << e;
                            i++;
                        }
                    }
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
                        it < file_content.end() && it + Magic::Events::Results::Auto::Record::size < file_content.end();
                        it += Magic::Events::Results::Auto::Record::size
                    ) {
                        std::array<uint8_t, Magic::Events::Results::Auto::Record::size> record_serialized { 0 };
                        std::copy(it, it + Magic::Events::Results::Auto::Record::size, record_serialized.begin());

                        std::array<uint8_t, sizeof(Magic::Events::Results::Auto::Timeval::T_RawData)> timeval_serialized;
                        std::copy(record_serialized.begin(), record_serialized.begin() + timeval_serialized.size(), timeval_serialized.begin());
                        std::array<uint8_t, sizeof(Magic::Events::Results::Auto::Point::T_RawData)> point_serialized;
                        std::copy(record_serialized.begin() + timeval_serialized.size(), record_serialized.end(), point_serialized.begin());

                        const auto timeval { Magic::Events::Results::Auto::Timeval::from_raw_data(timeval_serialized) };
                        const auto point { Magic::Events::Results::Auto::Point::from_raw_data(point_serialized) };
                        self.save_points.push(
                            {
                                .time = static_cast<double>(timeval.tv.tv_sec) + (static_cast<double>(timeval.tv.tv_usec) / 1'000'000.0),
                                .auto_meas = point
                            } 
                        );
                    }
                }

                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::End{} });
                self.status = Status::Off;
           } catch(const std::exception& e) {
                std::cout << "ERROR: GUI::Windows::RecordManager::download_cb: exception: " << e.what() << std::endl;
                self.status = Status::Error;
            }
        }

        void Auto::create_test_files_cb(std::stop_token st, Auto& self) {
            self.status = Status::CreatingTestFiles;
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::Start{} });
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::CreateTestFiles{} });
            const auto create_test_files_payload { self.shm->active_devices[self.index].information->read_for(boost::posix_time::milliseconds(60'000)) };

            if(create_test_files_payload.has_value() == false) {
                std::cout << "GUI::Windows::RecordManager::create_test_files_cb: timeout\n";
                self.status = Status::Error;
                return;
            }

            if(variant_tester<Magic::Events::Results::File::CreateTestFiles>(create_test_files_payload.value()) == false) {
                std::cout << "GUI::Windows::RecordManager::create_test_files_cb: create_test_files_payload: bad variant type\n";
                self.status = Status::Error;
                return;
            }

            if(
                [&]() {
                    bool ret { false };
                    std::visit([&](auto&& event) {
                        if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Events::Results::File::CreateTestFiles>) {
                            ret = event.status;
                        }
                    }, create_test_files_payload.value());
                    return ret;
                }() == false
            ) {
                self.status = Status::Error;
                std::cout << "GUI::Windows::RecordManager::create_test_files_cb: create_test_files_payload.value().status == false\n";
                return;
            }

            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::End{} });

            self.status = Status::Off;
            self.list();
        }

        void Auto::format_cb(std::stop_token st, Auto& self) {
            self.status = Status::Formatting;
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::Start{} });
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::Format{} });
            const auto format_payload { self.shm->active_devices[self.index].information->read_for(boost::posix_time::milliseconds(60'000)) };

            if(format_payload.has_value() == false) {
                std::cout << "GUI::Windows::RecordManager::format_cb: timeout\n";
                self.status = Status::Error;
                return;
            }

            if(variant_tester<Magic::Events::Results::File::Format>(format_payload.value()) == false) {
                std::cout << "GUI::Windows::RecordManager::format_cb: format_payload: bad variant type\n";
                self.status = Status::Error;
                return;
            }

            if(
                [&]() {
                    bool ret { false };
                    std::visit([&](auto&& event) {
                        if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Events::Results::File::Format>) {
                            ret = event.status;
                        }
                    }, format_payload.value());
                    return ret;
                }() == false
            ) {
                self.status = Status::Error;
                std::cout << "GUI::Windows::RecordManager::format_cb: format_payload.value().status == false\n";
                return;
            }

            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::End{} });
            self.status = Status::Off;
            self.list();
        }
    }
}