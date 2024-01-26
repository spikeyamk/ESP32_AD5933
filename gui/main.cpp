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
    constexpr size_t shm_size = 2 << 15;
    std::shared_ptr<BLE_Client::SHM::SHM> shm { BLE_Client::SHM::SHM::init() };
    boost::process::child ble_client { ble_client_path };

    /*
    bool done = false;
    GUI::run(done, ble_client, shm);
    */

    shm->send_unicmd_adapter(BLE_Client::StateMachines::Adapter::Events::turn_on{});
    std::this_thread::sleep_for(std::chrono::milliseconds(10'000));

    shm->send_unicmd_adapter(BLE_Client::StateMachines::Adapter::Events::start_discovery{});
    std::this_thread::sleep_for(std::chrono::milliseconds(10'000));

    shm->send_unicmd_adapter(BLE_Client::StateMachines::Adapter::Events::stop_discovery{});
    std::this_thread::sleep_for(std::chrono::milliseconds(10'000));

    if(ble_client.running()) {
        shm->send_unicmd_killer(BLE_Client::StateMachines::Killer::Events::kill{});
        std::this_thread::sleep_for(std::chrono::milliseconds(10'000));
    }
    ble_client.wait();
    return 0;
}
