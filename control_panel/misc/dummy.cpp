#include <thread>
#include <chrono>

int main() {
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
    }
}