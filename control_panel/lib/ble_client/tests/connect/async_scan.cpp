#include <trielo/trielo.hpp>

#include "ble_client/tests/tests.hpp"

int main() {
    return Trielo::trielo<BLE_Client::Tests::Connect::async_scan>(Trielo::OkErrCode(0));
}
