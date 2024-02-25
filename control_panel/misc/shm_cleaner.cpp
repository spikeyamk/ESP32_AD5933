#include "ble_client/shm/common/clean.hpp"

int main(int argc, char* argv[]) {
    (void) argc;
    BLE_Client::SHM::clean(boost::filesystem::path(argv[0]));
    return 0;
}