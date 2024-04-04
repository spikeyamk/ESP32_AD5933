#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>
#include <optional>

template <typename T>
class Channel {
    std::mutex mutex;
    std::condition_variable cv;
    std::queue<T> values;
public:
    Channel() = default;

    T read() {
        std::unique_lock lock(mutex);
        cv.wait(lock, [&]() { return values.empty() == false; });
        const T ret { values.front() };
        values.pop();
        return ret;
    }

    std::optional<T> try_read() {
        std::unique_lock lock(mutex, std::defer_lock);
        if(lock.try_lock() == false || values.empty()) {
            return std::nullopt;
        }

        const T ret { values.front() };
        values.pop();
        return ret;
    }

    void push(const T& obj) {
        std::scoped_lock lock(mutex);
        values.push(obj);
    }

    void clear() {
        std::scoped_lock lock(mutex);
        values.clear();
    }

    size_t size() {
        std::scoped_lock lock(mutex);
        return values.size();
    }
};