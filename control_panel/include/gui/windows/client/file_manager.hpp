#pragma once

#include <memory>
#include <vector>
#include <filesystem>
#include <optional>
#include <stop_token>

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
            std::optional<size_t> selected { std::nullopt };
            struct Bytes {
                uint64_t free { 0 };
                uint64_t total { 0 };
            };
            Bytes bytes {};
            float progress_bar_fraction { 0.0f };
        public:
            Status get_status() const;
            FileManager() = default;
            FileManager(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
            ~FileManager();
            void draw(bool& enable, const ImGuiID side_id, Lock& lock);
        private:
            void draw_table_rows();
            const std::optional<Lock> draw_inner();
            void list();
            void remove();
            void download();
        private:
            std::stop_source stop_source;
            static void list_cb(std::stop_token st, FileManager& self);
            static void download_cb(std::stop_token st, FileManager& self, const std::filesystem::path outPath);
        };
    }
}
