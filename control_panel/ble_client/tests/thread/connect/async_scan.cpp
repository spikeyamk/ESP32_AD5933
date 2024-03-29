#include <trielo/trielo.hpp>

#include "ble_client/tests/tests.hpp"

int main() {
    return Trielo::trielo<BLE_Client::Tests::Thread::Connect::async_scan>(Trielo::OkErrCode(0));
}