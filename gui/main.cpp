#include <thread>
#include <chrono>
#include <iostream>

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
    BLE_Client::SHM::Remover remover { BLE_Client::SHM::SHM::name, BLE_Client::SHM::SHM::cmd_deque_mutex_name, BLE_Client::SHM::SHM::cmd_deque_condition_name, BLE_Client::SHM::SHM::notify_deque_mutex_name, BLE_Client::SHM::SHM::notify_deque_condition_name };
    std::shared_ptr<BLE_Client::SHM::SHM> shm { BLE_Client::SHM::init_shm() };
    boost::process::child ble_client { ble_client_path };
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

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
