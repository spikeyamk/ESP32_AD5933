#pragma once

#include <optional>
#include <thread>

#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_condition.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread_time.hpp>

namespace BLE_Client {
    namespace SHM {
        template<typename T>
        using Deque = boost::interprocess::deque<T, boost::interprocess::allocator<T, boost::interprocess::managed_shared_memory::segment_manager>>;

        template<typename T>
        using DequeChannel = T_Channel<Deque<T>>;

        template<typename T>
        class TX_DequeChannel : public DequeChannel<T> {
        protected:
        public:
            TX_DequeChannel(const std::string_view& name, Deque<T>* deque) :
                DequeChannel<T> { name, deque }
            {}

            void send(const T& message) {
                this->data->push_back(message);
                this->condition.notify_one();
            }
        };

        template<typename T>
        class RX_DequeChannel : public DequeChannel<T> {
        protected:
        public:
            RX_DequeChannel(const std::string_view& name, Deque<T>* deque) :
                DequeChannel<T> { name, deque }
            {}

            T read() {
                boost::interprocess::scoped_lock lock(this->mutex);
                this->condition.wait(lock, [&]() { return !this->data->empty(); });
                const T ret = this->data->front();
                this->data->pop_front();
                return ret;
            }

            std::optional<T> read_for(const boost::posix_time::milliseconds& timeout) {
                boost::interprocess::scoped_lock lock(this->mutex);

                if(this->condition.timed_wait(lock, boost::get_system_time() + timeout, [&]() { return !this->data->empty(); }) == false) {
                    return std::nullopt;
                }

                if(this->data->empty()) {
                    return std::nullopt;
                }
                const T ret = this->data->front();
                this->data->pop_front();
                return ret;
            }

            void clear() {
                boost::interprocess::scoped_lock lock(this->mutex);
                this->data->clear();
            }
        };
    }
}

