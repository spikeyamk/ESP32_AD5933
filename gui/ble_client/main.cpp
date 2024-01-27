#include <iostream>
#include <memory>
#include <thread>
#include <simpleble/SimpleBLE.h>
#include <functional>
#include <stdexcept>
#include <typeinfo>
#include <cstdlib>
#include <typeinfo>
#include <vector>

#include <boost/thread/thread_time.hpp>
#include <trielo/trielo.hpp>

#include "ble_client/standalone/init.hpp"
#include "ble_client/standalone/shm.hpp"
#include "ble_client/standalone/esp32_ad5933.hpp"
#include "ble_client/standalone/state_machines/logger.hpp"
#include "ble_client/standalone/state_machines/killer/killer.hpp"
#include "ble_client/standalone/state_machines/adapter/adapter.hpp"
#include "ble_client/standalone/state_machines/connector/connector.hpp"
#include "ble_client/standalone/state_machines/connection/connection.hpp"

void checker(
	std::stop_source stop_source,
	BLE_Client::StateMachines::Adapter::T_StateMachine& adapter_sm,
	SimpleBLE::Adapter& adapter,
	std::shared_ptr<BLE_Client::SHM::ChildSHM> shm
) {
	std::stop_token st = stop_source.get_token();
	using namespace boost::sml;
	while(st.stop_requested() == false) {
		if(adapter_sm.is(state<BLE_Client::StateMachines::Adapter::States::off>) == false) {
			try {
				if(adapter.bluetooth_enabled() == false) {
					adapter_sm.set_current_states(state<BLE_Client::StateMachines::Adapter::States::off>);
				}
			} catch(const std::exception& e) {
				std::cerr << "ERROR: BLE_Client::checker: exception: " << e.what() << std::endl;
				adapter_sm.set_current_states(state<BLE_Client::StateMachines::Adapter::States::off>);
			}
		}

		adapter_sm.visit_current_states([&](auto&& visited_state) {
			try {
				*(shm->active_state) = BLE_Client::StateMachines::Adapter::States::stupid_sml.at(visited_state.c_str());
			} catch(const std::exception& e) {
				std::cerr << "ERROR: BLE_Client::checker: exception: " << e.what() << std::endl;
				std::cerr << "ERROR: This compiler gives the visited_state a different prefix\n";
				std::exit(-1);
			}
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void listen_to_unicmds(
	std::stop_source stop_source,
	std::shared_ptr<BLE_Client::SHM::ChildSHM> shm,
	BLE_Client::StateMachines::Killer::T_StateMachine& killer,
	BLE_Client::StateMachines::Adapter::T_StateMachine& adapter_sm,
	std::vector<decltype(BLE_Client::StateMachines::Connection::Dummy<int>::sm)*>& connections,
	SimpleBLE::Adapter& simpleble_adapter,
	BLE_Client::StateMachines::Connector::T_StateMachine& connector
) {
	using namespace boost::interprocess;
	std::stop_token st { stop_source.get_token() };
	while(st.stop_requested() == false) {
		auto cmd { shm->cmd.read_for(boost::posix_time::milliseconds(100)) };
		if(cmd.has_value()) {
			std::visit([&killer, &adapter_sm, &connections, shm, &simpleble_adapter, &connector](auto&& event_variant) {
				using T_EventVariantDecay = std::decay_t<decltype(event_variant)>;
				if constexpr (std::is_same_v<T_EventVariantDecay, BLE_Client::StateMachines::Killer::Events::T_Variant>) {
					std::visit([&killer](auto&& event) {
						killer.process_event(event);
					}, event_variant);
				} else if constexpr (std::is_same_v<T_EventVariantDecay, BLE_Client::StateMachines::Adapter::Events::T_Variant>) {
					std::visit([&adapter_sm](auto&& event) {
						adapter_sm.process_event(event);
					}, event_variant);
				} else if constexpr (std::is_same_v<T_EventVariantDecay, BLE_Client::StateMachines::Connector::Events::connect>) {
					connector.process_event(event_variant);
				} else if constexpr (std::is_same_v<T_EventVariantDecay, BLE_Client::StateMachines::Connection::Events::T_Variant>) {
					std::visit([&connections](auto&& event) {
						using T_EventDecay = std::decay_t<decltype(event)>;
						connections[event.index]->process_event(event);
						if constexpr (std::is_same_v<T_EventDecay, BLE_Client::StateMachines::Connection::Events::disconnect>) {
							delete connections[event.index];
							connections.erase(connections.begin() + event.index);
						}
					}, event_variant);
				}
			}, cmd.value());
		}
	}
}

int main(void) {
    std::printf("BLE_Client: process started\n");
	std::atexit([]() { std::printf("BLE_Client: process finished\n"); });
	auto shm = std::make_shared<BLE_Client::SHM::ChildSHM>();

	std::stop_source stop_source;
	SimpleBLE::Adapter simpleble_adapter;
	BLE_Client::StateMachines::Logger killer_logger;
	BLE_Client::StateMachines::Killer::T_StateMachine killer { stop_source, killer_logger };
	BLE_Client::StateMachines::Logger adapter_logger;
	BLE_Client::StateMachines::Adapter::T_StateMachine adapter_sm { simpleble_adapter, shm, adapter_logger };
	std::vector<decltype(BLE_Client::StateMachines::Connection::Dummy<int>::sm)*> connections;
	BLE_Client::StateMachines::Connector::T_StateMachine connector { simpleble_adapter, shm, connections, adapter_logger };

	std::jthread listener_thread(listen_to_unicmds, stop_source, shm, std::ref(killer), std::ref(adapter_sm), std::ref(connections), std::ref(simpleble_adapter), std::ref(connector));
    std::jthread checker_thread(checker, stop_source, std::ref(adapter_sm), std::ref(simpleble_adapter), shm);

    return 0;
}
