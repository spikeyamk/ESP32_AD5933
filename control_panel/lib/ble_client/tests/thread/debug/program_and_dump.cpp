#include <trielo/trielo.hpp>

#include "ble_client/tests/tests.hpp"

int main() {
    return Trielo::trielo<BLE_Client::Tests::Thread::Debug::program_and_dump>(Trielo::OkErrCode(0));
}