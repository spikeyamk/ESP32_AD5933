#include <iostream>
#include <memory>
#include <thread>
#include <simpleble/SimpleBLE.h>
#include <functional>
#include <stdexcept>
#include <typeinfo>
#include <cstdlib>
#include <typeinfo>

#include <boost/thread/thread_time.hpp>
#include <trielo/trielo.hpp>

#include "ble_client/standalone/init.hpp"
#include "ble_client/standalone/shm.hpp"
#include "ble_client/standalone/state_machine.hpp"
#include "ble_client/standalone/shm_state_machine.hpp"

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

int main(void) {
    std::printf("BLE_Client: process started\n");
	std::atexit([]() { std::printf("BLE_Client: process finished\n"); });
    std::shared_ptr<BLE_Client::SHM::SHM> shm = std::make_shared<BLE_Client::SHM::SHM>();
	shm->attach();

    SimpleBLE::Adapter adapter;
    BLE_Client::Discovery::my_logger logger;
	std::stop_source stop_source;
	SimpleBLE::Peripheral peripheral;
	BLE_Client::ESP32_AD5933 esp32_ad5933;
    BLE_Client::Discovery::T_StateMachine discovery_agent { adapter, logger, shm, stop_source, peripheral, esp32_ad5933 };

    std::jthread listener_thread(listen_to_cmds, stop_source, shm, std::ref(discovery_agent));
    std::jthread checker_thread(checker, stop_source, std::ref(discovery_agent), std::ref(adapter), std::ref(peripheral), shm);

    return 0;
}
