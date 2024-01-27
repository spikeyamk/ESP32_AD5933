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
    const boost::filesystem::path ble_client_path {
    #ifdef _MSC_VER
        "C:/Users/janco/source/repos/ESP32_AD5933/gui/build/ble_client/Debug/ble_client.exe"
    #else
        "/home/spikeyamk/Documents/git-repos/ESP32_AD5933/gui/build/ble_client/ble_client"
    #endif
    };
    auto shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
    boost::process::child ble_client { ble_client_path };

    bool done = false;
    GUI::run(done, ble_client, shm);

    if(ble_client.running()) {
        shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
        std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
        ble_client.terminate();
    }
    return 0;
}
