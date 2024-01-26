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
#include "ble_client/standalone/state_machine.hpp"
#include "ble_client/standalone/esp32_ad5933.hpp"
#include "ble_client/standalone/state_machines/logger.hpp"
#include "ble_client/standalone/state_machines/killer/killer.hpp"
#include "ble_client/standalone/state_machines/adapter/adapter.hpp"
#include "ble_client/standalone/state_machines/connection/connection.hpp"

void listen_to_cmds(std::stop_source stop_source, std::shared_ptr<BLE_Client::SHM::SHM> shm, BLE_Client::Discovery::T_StateMachine& discovery_agent) {
	using namespace boost::interprocess;
	std::stop_token st = stop_source.get_token();
	while(st.stop_requested() == false) {
		scoped_lock<named_mutex> lock(shm->cmd_deque_mutex);
		shm->cmd_deque_condition.timed_wait(
			lock,
            boost::get_system_time() + boost::posix_time::milliseconds(100),
			[&shm]() { return !shm->cmd_deque->empty(); }
		);

		while(shm->cmd_deque->empty() == false) {
			std::visit([&discovery_agent](auto&& event) {
				discovery_agent.process_event(event);
			}, shm->cmd_deque->front());
			shm->cmd_deque->pop_front();
		}
	}
}

void checker(std::stop_source stop_source, BLE_Client::Discovery::T_StateMachine& state_machine, SimpleBLE::Adapter& adapter, SimpleBLE::Peripheral& peripheral, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
	std::stop_token st = stop_source.get_token();
	using namespace boost::sml;
	while(st.stop_requested() == false) {
		if(state_machine.is(state<BLE_Client::Discovery::States::off>, state<BLE_Client::Discovery::States::dead>) == false) {
			try {
				if(adapter.bluetooth_enabled() == false) {
					state_machine.set_current_states(state<BLE_Client::Discovery::States::off>, state<BLE_Client::Discovery::States::alive>);
				}
			} catch(const std::exception& e) {
				std::cerr << "ERROR: BLE_Client::checker: exception: " << e.what() << std::endl;
				state_machine.set_current_states(state<BLE_Client::Discovery::States::off>, state<BLE_Client::Discovery::States::alive>);
			}
		}

		if(state_machine.is(state<BLE_Client::Discovery::States::connected>, state<BLE_Client::Discovery::States::alive>)) {
			try {
				if(peripheral.is_connected() == false) {
					state_machine.set_current_states(state<BLE_Client::Discovery::States::off>, state<BLE_Client::Discovery::States::alive>);
				}
			} catch(const std::exception& e) {
				state_machine.set_current_states(state<BLE_Client::Discovery::States::off>, state<BLE_Client::Discovery::States::alive>);
				std::cerr << "ERROR: BLE_Client::checker: exception: " << e.what() << std::endl;
			}
		}

		state_machine.visit_current_states([&](auto&& visited_state) {
			if((visited_state.c_str() == BLE_Client::Discovery::States::prefix + "alive")
			|| (visited_state.c_str() == BLE_Client::Discovery::States::prefix + "dead")) {
				return;
			}
			try {
				*(shm->active_state) = BLE_Client::Discovery::States::stupid_sml.at(visited_state.c_str());
			} catch(const std::exception& e) {
				std::cerr << "ERROR: BLE_Client::checker: exception: " << e.what() << std::endl;
				std::cerr << "ERROR: This compiler gives the visited_state a different prefix\n";
				std::exit(-1);
			}
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void listen_to_unicmds(std::stop_source stop_source, BLE_Client::SHM::SHM* shm, BLE_Client::StateMachines::Killer::T_StateMachine& killer, BLE_Client::StateMachines::Adapter::T_StateMachine& adapter, std::vector<std::shared_ptr<BLE_Client::StateMachines::Connection::T_StateMachine>>& connections, SimpleBLE::Adapter& simpleble_adapter) {
	using namespace boost::interprocess;
	std::stop_token st { stop_source.get_token() };
	while(st.stop_requested() == false) {
		scoped_lock<named_mutex> lock(shm->unicmd_deque_mutex);
		shm->unicmd_deque_condition.timed_wait(
			lock,
            boost::get_system_time() + boost::posix_time::milliseconds(100),
			[&shm]() { return !shm->unicmd_deque->empty(); }
		);

		while(shm->unicmd_deque->empty() == false) {
			std::visit([&killer, &adapter, &connections, shm, &simpleble_adapter](auto&& event_variant) {
				using T_EventVariantDecay = std::decay_t<decltype(event_variant)>;
				if constexpr (std::is_same_v<T_EventVariantDecay, BLE_Client::StateMachines::Killer::Events::T_Variant>) {
					std::visit([&killer](auto&& event) {
						killer.process_event(event);
					}, event_variant);
				} else if constexpr (std::is_same_v<T_EventVariantDecay, BLE_Client::StateMachines::Adapter::Events::T_Variant>) {
					std::visit([&adapter](auto&& event) {
						adapter.process_event(event);
					}, event_variant);
				} else if constexpr (std::is_same_v<T_EventVariantDecay, BLE_Client::StateMachines::Connection::Events::T_Variant>) {
					std::visit([&connections, shm, &adapter, &simpleble_adapter](auto&& event) {
						if(connections.size() < event.index) {
							SimpleBLE::Peripheral peripheral;
							BLE_Client::ESP32_AD5933 esp32_ad5933;
							BLE_Client::StateMachines::Logger logger;
							connections.push_back(std::make_shared<BLE_Client::StateMachines::Connection::T_StateMachine>(logger, shm, simpleble_adapter, peripheral, esp32_ad5933));
						}
						connections[event.index]->process_event(event);
					}, event_variant);
				}
			}, shm->unicmd_deque->front());
			shm->unicmd_deque->pop_front();
		}
	}
}

int main(void) {
    std::printf("BLE_Client: process started\n");
	std::atexit([]() { std::printf("BLE_Client: process finished\n"); });
    BLE_Client::SHM::SHM* shm { BLE_Client::SHM::SHM::attach() };

	std::stop_source stop_source;
	SimpleBLE::Adapter simpleble_adapter;
	BLE_Client::StateMachines::Logger killer_logger;
	BLE_Client::StateMachines::Killer::T_StateMachine killer { stop_source, killer_logger };
	BLE_Client::StateMachines::Logger adapter_logger;
	BLE_Client::StateMachines::Adapter::T_StateMachine adapter { simpleble_adapter, shm, adapter_logger };
	std::vector<std::shared_ptr<BLE_Client::StateMachines::Connection::T_StateMachine>> connections;

	std::jthread listener_thread(listen_to_unicmds, stop_source, shm, std::ref(killer), std::ref(adapter), std::ref(connections), std::ref(simpleble_adapter));

	/*
	SimpleBLE::Peripheral peripheral;
	BLE_Client::ESP32_AD5933 esp32_ad5933;
    BLE_Client::Discovery::T_StateMachine discovery_agent { adapter, logger, shm, stop_source, peripheral, esp32_ad5933 };

    std::jthread listener_thread(listen_to_cmds, stop_source, shm, std::ref(discovery_agent));
    std::jthread checker_thread(checker, stop_source, std::ref(discovery_agent), std::ref(adapter), std::ref(peripheral), shm);
	*/

    return 0;
}
