#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <deque>
#include <mutex>

template<typename T>
class Channel {
private:
    std::mutex mutex;
    T data;
public:
    void send(const T x) {
        data = x;
    }

    T read() {
        std::scoped_lock lock(mutex);
        return data;
    }
};
