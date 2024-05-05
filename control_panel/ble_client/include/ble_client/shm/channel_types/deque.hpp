#pragma once

#include <optional>
#include <chrono>
#include <deque>

#include "ble_client/shm/channel_types/base.hpp"

namespace BLE_Client {
    namespace SHM {
        template <typename T>
        class DequeChannel : public T_Channel<std::deque<T>> {
            using Base = T_Channel<std::deque<T>>;
        public:
            DequeChannel() :
                Base {}
            {}

            void send(const T& message) {
                std::scoped_lock lock(this->mutex);
                this->data.push_back(message);
                this->condition.notify_one();
            }

            T read() {
                std::unique_lock lock(this->mutex);
                this->condition.wait(lock, [&]() { return !this->data.empty(); });
                const T ret { this->data.front() };
                this->data.pop_front();
                return ret;
            }

            std::optional<T> read_for(const std::chrono::milliseconds& timeout) {
                std::unique_lock lock(this->mutex);

                if(this->condition.wait_for(lock, timeout, [&]() { return !this->data.empty(); }) == false) {
                    return std::nullopt;
                }

                if(this->data.empty()) {
                    return std::nullopt;
                }
                const T ret = this->data.front();
                this->data.pop_front();
                return ret;
            }

            void clear() {
                std::scoped_lock lock(this->mutex);
                this->data.clear();
            }
        };
    }
}

