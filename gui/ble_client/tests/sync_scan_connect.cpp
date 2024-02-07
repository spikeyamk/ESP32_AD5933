#include <trielo/trielo.hpp>

#include "ble_client/tests/tests.hpp"

int main() {
    return Trielo::trielo<BLE_Client::Tests::sync_scan_connect>(Trielo::OkErrCode(0));
}