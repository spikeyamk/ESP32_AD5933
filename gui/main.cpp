#include <thread>
#include <chrono>
#include <iostream>

#include <boost/process.hpp>

#include "ble_client/ble_client.hpp"
#include "gui/imgui_sdl.hpp"
#include "magic/packets.hpp"

#include "ble_client/standalone/shm.hpp"
#include "ble_client/standalone/test.hpp"

void gui() {
    bool done = false;
    std::jthread gui_thread(GUI::run, std::ref(done));
    GUI::run(done);
}

int main(int argc, char* argv[]) {
    #ifdef _MSC_VER
        const boost::filesystem::path client_path { "C:/Users/janco/source/repos/ESP32_AD5933/gui/build/ble_client/Debug/ble_client.exe" };
    #else
        const boost::filesystem::path client_path { "/home/spikeyamk/Documents/git-repos/ESP32_AD5933/gui/build/ble_client/ble_client" };
    #endif
    BLE_Client::SHM::Remover remover { BLE_Client::SHM::SHM::name, BLE_Client::SHM::SHM::cmd_deque_mutex_name, BLE_Client::SHM::SHM::cmd_deque_condition_name, BLE_Client::SHM::SHM::notify_deque_mutex_name, BLE_Client::SHM::SHM::notify_deque_condition_name };
    std::shared_ptr<BLE_Client::SHM::SHM> shm { BLE_Client::SHM::init_shm() };
    boost::process::child client { client_path };

    BLE_Client::test(client, shm);

    client.wait();
    return 0;
}
