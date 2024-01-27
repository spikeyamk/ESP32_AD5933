#include <boost/thread/thread_time.hpp>
#include <trielo/trielo.hpp>
#include <simpleble/SimpleBLE.h>

#include "ble_client/standalone/test.hpp"
#include "ble_client/standalone/init.hpp"
#include "ble_client/standalone/shm.hpp"
#include "ble_client/standalone/cmd_listener.hpp"
#include "ble_client/standalone/esp32_ad5933.hpp"
#include "ble_client/standalone/state_machines/logger.hpp"
#include "ble_client/standalone/state_machines/killer/killer.hpp"
#include "ble_client/standalone/state_machines/adapter/adapter.hpp"
#include "ble_client/standalone/state_machines/adapter/checker.hpp"
#include "ble_client/standalone/state_machines/connector/connector.hpp"
#include "ble_client/standalone/state_machines/connection/connection.hpp"

#include "ble_client/standalone/worker.hpp"

namespace BLE_Client {
    void worker() {
        auto child_shm { std::make_shared<BLE_Client::SHM::ChildSHM>() };
        SimpleBLE::Adapter simpleble_adapter;
        BLE_Client::StateMachines::Logger killer_logger;
        std::stop_source stop_source;
        BLE_Client::StateMachines::Killer::T_StateMachine killer { stop_source, killer_logger };
        BLE_Client::StateMachines::Logger adapter_logger;
        BLE_Client::StateMachines::Adapter::T_StateMachine adapter_sm { simpleble_adapter, child_shm, adapter_logger };
        std::vector<decltype(BLE_Client::StateMachines::Connection::Dummy<int>::sm)*> connections;
        BLE_Client::StateMachines::Connector::T_StateMachine connector { simpleble_adapter, child_shm, connections, adapter_logger };
        
        std::jthread cmd_listener_thread(BLE_Client::cmd_listener, stop_source, child_shm, std::ref(killer), std::ref(adapter_sm), std::ref(connections), std::ref(simpleble_adapter), std::ref(connector));
        std::jthread checker_thread(BLE_Client::StateMachines::Adapter::checker, stop_source, std::ref(adapter_sm), std::ref(simpleble_adapter), child_shm);
    }
}