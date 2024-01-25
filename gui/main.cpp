#include <thread>
#include <chrono>
#include <iostream>
#include <stop_token>

#include <boost/process.hpp>

#include "ble_client/ble_client.hpp"
#include "gui/imgui_sdl.hpp"
#include "magic/packets.hpp"

#include "ble_client/standalone/shm.hpp"
#include "ble_client/standalone/test.hpp"

int main(int argc, char* argv[]) {
    #ifdef _MSC_VER
        const boost::filesystem::path ble_client_path { "C:/Users/janco/source/repos/ESP32_AD5933/gui/build/ble_client/Debug/ble_client.exe" };
    #else
        const boost::filesystem::path ble_client_path { "/home/spikeyamk/Documents/git-repos/ESP32_AD5933/gui/build/ble_client/ble_client" };
    #endif
    BLE_Client::SHM::Remover remover {};
    constexpr size_t shm_size = 2 << 15;
    std::shared_ptr<BLE_Client::SHM::SHM> shm = std::make_shared<BLE_Client::SHM::SHM>(shm_size);
    shm->init();
    boost::process::child ble_client { ble_client_path };

    bool done = false;
    GUI::run(done, ble_client, shm);

    if(ble_client.running()) {
        shm->cmd_deque->push_back(BLE_Client::Discovery::Events::kill{});
    }
    if(ble_client.wait_for(std::chrono::milliseconds(5'000)) == false) {
        ble_client.terminate();
    }
    return 0;
}
