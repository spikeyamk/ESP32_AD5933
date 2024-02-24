#pragma once

#include <memory>
#include <vector>
#include <filesystem>
#include <optional>

#include <nfd.hpp>
#include "imgui.h"

#include "ble_client/shm/parent/parent.hpp"
#include "gui/windows/client/lock.hpp"

namespace GUI {
    namespace Windows {
        class FileManager {
        public:
            static constexpr std::u8string_view name_base { u8"File Manager##" };
        private:
            size_t index;
            std::u8string name { name_base };
            bool first { true };
            std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };
        public:
            enum class Status {
                NotListed,
                Listing,
                Listed,
                Downloading,
                Failed,
            };
        private:
            Status status { Status::NotListed };
            struct ListTable {
                std::vector<Magic::Events::Results::File::List> paths;
                std::vector<Magic::Events::Results::File::Size> sizes;
            };
            ListTable list_table {};
            int selected = -1;
        public:
            Status get_status() const;
            FileManager() = default;
            FileManager(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
            void draw(bool& enable, const ImGuiID side_id, std::optional<Lock>& lock);
        private:
            void draw_inner();
            void list();
            void remove();
            void download();
        private:
            static void list_cb(FileManager& self);
            static void download_cb(FileManager& self, const std::filesystem::path outPath);
        };
    }
}
