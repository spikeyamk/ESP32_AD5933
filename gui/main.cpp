#include <thread>
#include <chrono>
#include <iostream>
#include <stop_token>

#include <boost/process.hpp>

#include "gui/imgui_sdl.hpp"

#include "ble_client/standalone/shm.hpp"
#include "ble_client/standalone/test.hpp"

int main(int argc, char* argv[]) {
    const boost::filesystem::path ble_client_path {
    #ifdef _MSC_VER
        "C:/Users/janco/source/repos/ESP32_AD5933/gui/build/ble_client/Debug/ble_client.exe"
    #else
        "/home/spikeyamk/Documents/git-repos/ESP32_AD5933/gui/build/ble_client/ble_client"
    #endif
    };

    std::shared_ptr<BLE_Client::SHM::ParentSHM> shm = nullptr;
    try {
        shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
    } catch(const boost::interprocess::interprocess_exception& e) {
        std::cout << "ERROR: GUI: Failed to open SHM: exception: " << e.what() << std::endl;
        {
            BLE_Client::SHM::Cleaner cleaner {};
        }
        try {
            shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
        } catch(const boost::interprocess::interprocess_exception& e) {
            std::cout << "ERROR: GUI: Failed to open SHM even after cleaning: exception: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    boost::process::child ble_client { ble_client_path };

    bool done = false;
    GUI::run(done, ble_client, shm);

    return 0;
}
