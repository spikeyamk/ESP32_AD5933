#pragma once

#include <mutex>
#include <condition_variable>

namespace BLE_Client {
    namespace SHM {
        template<typename T_Container>
        class T_Channel {
        protected:
            T_Container data;
            std::mutex mutex;
            std::condition_variable condition;
        public:
            T_Channel() = default;
        };
    }
}