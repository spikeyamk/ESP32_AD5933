#include <iostream>
#include <thread>

#include "gui/run.hpp"
#include "ble_client/child_main.hpp"

int main(int argc, char* argv[]) {
    std::atexit([]() { std::cout << "Control Panel: GUI: Parent process finished\n"; });
    std::shared_ptr<BLE_Client::SHM::SHM> shm { std::make_shared<BLE_Client::SHM::SHM>() };
    std::jthread ble_client { BLE_Client::child_main, shm };
    bool done { false };
    GUI::run(done, shm);
    return EXIT_SUCCESS;
}