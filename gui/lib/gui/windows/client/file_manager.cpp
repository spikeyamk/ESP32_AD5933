#include <string>
#include <thread>
#include <fstream>
#include <stdexcept>
#include <cmath>

#include "imgui_internal.h"
#include <utf/utf.hpp>

#include "imgui_custom/input_items.hpp"
#include "imgui_custom/spinner.hpp"
#include "misc/variant_tester.hpp"
#include "gui/boilerplate.hpp"

#include "gui/windows/client/file_manager.hpp"

namespace GUI {
    namespace Windows {
        FileManager::Status FileManager::get_status() const {
            return status;
        }

        FileManager::FileManager(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) :
            index { index },
            shm{ shm }
        {
            name.append(utf::as_u8(std::to_string(index)));
        }

        void FileManager::draw_inner() {
            if(status == Status::Listing || status == Status::Downloading) {
                ImGui::BeginDisabled();
                ImGui::Button("List");
                ImGui::EndDisabled();
                ImGui::SameLine();
                const float scale = GUI::Boilerplate::get_scale();
                Spinner::Spinner("Processing", 5.0f * scale, 2.0f * scale, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
            } else {
                if(ImGui::Button("List")) {
                    list();
                }
            }

            if(ImGui::BeginTable("List Table", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Path");
                ImGui::TableNextColumn();
                ImGui::Text("Size [Bytes]");
                if(status == Status::Listed) {
                    for(size_t i = 0; i < list_table.paths.size() && i < list_table.sizes.size(); i++) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::PushID(static_cast<int>(i));
                        if(ImGui::Selectable(std::string(list_table.paths[i].path.begin(), list_table.paths[i].path.end()).c_str(), static_cast<int>(i) == selected, ImGuiSelectableFlags_SpanAllColumns)) {
                            if(selected == static_cast<int>(i)) {
                                selected = -1;
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
            }
            ImGui::EndTable();

            if(selected != -1) {
                if(status == Status::Listing || status == Status::Downloading) {
                    ImGui::BeginDisabled();
                    ImGui::Button("Download");
                    ImGui::SameLine();
                    ImGui::Button("Remove");
                    ImGui::EndDisabled();
                } else {
                    if(ImGui::Button("Download")) {
                        download();
                    }
                    if(ImGui::Button("Remove")) {
                        remove();
                    }
                }
            }


        }

        void FileManager::draw(bool& enable, const ImGuiID side_id, std::optional<Lock>& lock) {
            if(first) {
                ImGui::DockBuilderDockWindow((const char*) name.c_str(), side_id);
            }

            if(ImGui::Begin((const char*) name.c_str(), &enable) == false) {
                ImGui::End();
                return;
            }

            if(first) {
                ImGui::End();
                first = false;
                return;
            }

            if(lock.has_value() && lock.value() != Lock::FileManager) {
                ImGui::BeginDisabled();
                draw_inner();
                ImGui::EndDisabled();
            } else {
                draw_inner();
            }

            ImGui::End();
        }

        void FileManager::list_cb(FileManager& self) {
            self.status = Status::Listing;
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::Start{} });
            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::ListCount{} });
            const auto list_count_payload { self.shm->active_devices[self.index].information->read_for(boost::posix_time::milliseconds(5'000)) };

            if(list_count_payload.has_value() == false) { 
                self.status = Status::Failed;
                std::cout << "ERROR: GUI::Windows::FileManager::list_cb: failed to retreive list_count: timeout\n";
                return;
            }
            
            if(variant_tester<Magic::Events::Results::File::ListCount>(list_count_payload.value()) == false) {
                self.status = Status::Failed;
                std::cout << "ERROR: GUI::Windows::FileManager::list_cb: failed to retreive list_count: wrong variant type\n";
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
                std::cout << "ERROR: GUI::Windows::FileManager::list_cb: failed to retreive list_count: list_cout empty\n";
                self.status = Status::NotListed;
            }

            self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::List{} });
            std::vector<Magic::Events::Results::File::List> listed_paths;
            listed_paths.reserve(list_count.num_of_files);
            while(listed_paths.size() != list_count.num_of_files) {
                const auto list_payload { self.shm->active_devices[self.index].information->read_for(boost::posix_time::milliseconds(5'000)) };
                if(list_payload.has_value() == false) {
                    std::cout << "ERROR: GUI::Windows::FileManager::list_cb: failed to retreive list_path: timeout\n";
                    self.status = Status::Failed;
                    return;
                }

                if(variant_tester<Magic::Events::Results::File::List>(list_payload.value()) == false) {
                    std::cout << "ERROR: GUI::Windows::FileManager::list_cb: failed to retreive list_path: wrong variant type\n";
                    self.status = Status::Failed;
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
                    std::cout << "ERROR: GUI::Windows::FileManager::list_cb: failed to retreive list_size: timeout\n";
                    self.status = Status::Failed;
                    return;
                }

                if(variant_tester<Magic::Events::Results::File::Size>(size_payload.value()) == false) {
                    std::cout << "ERROR: GUI::Windows::FileManager::list_cb: failed to retreive list_size: wrong variant\n";
                    self.status = Status::Failed;
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

            self.status = Status::Listed;
        }

        void FileManager::list() {
            std::jthread t1(list_cb, std::ref(*this));
            t1.detach();
        }

        void FileManager::remove() {
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::File::Start{} });
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::File::Remove::from_raw_data(list_table.paths[selected].to_raw_data()) });
            shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ index, Magic::Events::Commands::File::End{} });
            list();
        }

        void FileManager::download() {
            nfdchar_t* path = nullptr;
            const nfdresult_t result = NFD::SaveDialog(path, nullptr, 0, nullptr, std::string(list_table.paths[selected].path.begin(), list_table.paths[selected].path.end()).c_str());
            if(result == NFD_OKAY) {
                std::jthread t1(download_cb, std::ref(*this), std::filesystem::path(path));
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

        void FileManager::download_cb(FileManager& self, const std::filesystem::path path) {
            try {
                self.status = Status::Downloading;
                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::Start{} });
                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::Download::from_raw_data(self.list_table.paths[self.selected].to_raw_data()) });
                std::vector<Magic::Events::Results::File::Download> download_slices;
                const size_t wished_size = static_cast<size_t>(std::ceil(static_cast<float>(self.list_table.sizes[self.selected].num_of_bytes) / static_cast<float>(sizeof(Magic::T_MaxDataSlice))));
                download_slices.reserve(wished_size);
                while(download_slices.size() < wished_size) {
                    const auto download_payload { self.shm->active_devices[self.index].information->read_for(boost::posix_time::milliseconds(5'000)) };
                    if(download_payload.has_value() == false) {
                        std::cout << "ERROR: GUI::Windows::FileManager::download_cb: failed to retreive download_payload: timeout\n";
                        self.status = Status::Failed;
                        return;
                    }

                    if(variant_tester<Magic::Events::Results::File::Download>(download_payload.value()) == false) {
                        std::cout << "ERROR: GUI::Windows::FileManager::download_cb: failed to retreive download_payload: wrong variant type\n";
                        self.status = Status::Failed;
                        return;
                    }

                    std::visit([&download_slices](auto&& event) {
                        if constexpr(std::is_same_v<std::decay_t<decltype(event)>, Magic::Events::Results::File::Download>) {
                            download_slices.push_back(event);
                        }
                    }, download_payload.value());
                }

                std::ofstream file(path, std::ios::out | std::ios::binary);
                for(const auto& slice: download_slices) {
                    for(const auto& e: slice.slice) {
                        file << e;
                    }
                }

                self.shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ self.index, Magic::Events::Commands::File::End{} });
                self.status = Status::Listed;
           } catch(const std::exception& e) {
                std::cout << "ERROR: GUI::Windows::FileManager::download_cb: exception: " << e.what() << std::endl;
                self.status = Status::Failed;
            }
        }
    }
}
