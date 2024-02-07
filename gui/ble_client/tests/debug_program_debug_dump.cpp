#include <trielo/trielo.hpp>

#include "ble_client/tests/tests.hpp"

int main() {
    return Trielo::trielo<BLE_Client::Tests::debug_program_debug_dump>(Trielo::OkErrCode(0));
}