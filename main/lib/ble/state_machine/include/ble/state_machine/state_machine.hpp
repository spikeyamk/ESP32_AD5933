#pragma once

#include <iostream>
#include <queue>
#include <mutex>
#include <functional>
#include <thread>
#include <chrono>

#include <boost/sml.hpp>

#include "ble/state_machine/sender.hpp"

namespace BLE {
    namespace sml = boost::sml;
	namespace States {
		struct off {};
		struct on {};
		struct advertising {};
		struct connected {};

		struct debug;
		namespace FreqSweep {
			struct configured {};
			struct running {};
		}
	}

	namespace Events {
		struct turn_on{};
		struct connect {};

		struct disconnect {};

		namespace Debug {
			struct start {};
			struct end {};
			struct dump_all_registers {};
			struct program_all_registers {};
			struct ctrlHB_command {
				uint8_t ctrlHB_command_or_mask;
				ctrlHB_command() = delete;
				constexpr ctrlHB_command(const uint8_t command) :
					ctrlHB_command_or_mask { command }
				{}
            };
		}

		namespace FreqSweep {
			struct configure {};
			struct start {};
			struct end {};
			namespace Private {
				struct complete {}; //Commented out because I want to make it inaccessible to other translation units
			}
		};
	}

	namespace Actions {
		static void turn_on() {
			std::printf("BLE::StateMachine::Actions::turn_on\n");
		};

		static void advertise() {
			std::printf("BLE::StateMachine::Actions::advertise\n");
		};

		static void connect() {
			std::printf("BLE::StateMachine::Actions::connect\n");
		};

		static void disconnect() {
			std::printf("BLE::StateMachine::Actions::disconnect\n");
		};

		namespace Debug {
			static const char *namespace_name = "Debug::";
			static void start() {
				std::printf("BLE::StateMachine::Actions::%sdisconnect\n", namespace_name);
			}

			static void end() {
				std::printf("BLE::StateMachine::Actions::%send\n", namespace_name);
			}

			static void dump_all_registers() {
				std::printf("BLE::StateMachine::Actions::%sdump_all_registers\n", namespace_name);
			}

			static void program_all_registers() {
				std::printf("BLE::StateMachine::Actions::%sprogram_all_registers\n", namespace_name);
			}

			static void ctrlHB_command(const Events::Debug::ctrlHB_command &event) {
				std::printf("BLE::StateMachine::Actions::%sctrlHB_command\n", namespace_name);
			}
		}

		namespace FreqSweep {
			static const char *namespace_name = "FreqSweep::";
			static void configure() {
				std::printf("BLE::StateMachine::Actions::%sconfigure\n", namespace_name);
			}

			static void run(Sender &sender, sml::back::process<Events::FreqSweep::Private::complete> process_event) {
				std::unique_lock lock(sender.mutex);
				std::printf("BLE::StateMachine::Actions::%srun\n", namespace_name);
				for(int i = 0; i < 10; i++) {
					std::printf("BLE::StateMachine::Actions::%srun: sending %d\n", namespace_name, i);
					sender.send(i);
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
				process_event(Events::FreqSweep::Private::complete{});
			} 
			static void end() {
				std::printf("BLE::StateMachine::Actions::%send\n", namespace_name);
			}
		}
	}

	struct Connection {
		auto operator()() const {
			using namespace boost::sml;
			using namespace std;
			auto ret = make_transition_table(
				*state<States::off>        + event<Events::turn_on> / function{Actions::turn_on}   = state<States::on>,

				state<States::on>           						/ function{Actions::advertise} = state<States::advertising>,
				state<States::advertising> + event<Events::connect> / function{Actions::connect}   = state<States::connected>,

				state<States::connected> + event<Events::disconnect>           / function{Actions::disconnect}           = state<States::advertising>,
				state<States::connected> + event<Events::Debug::start>         / function{Actions::Debug::start}         = state<States::debug>,
				state<States::connected> + event<Events::FreqSweep::configure> / function{Actions::FreqSweep::configure} = state<States::FreqSweep::configured>,

				state<States::debug> + event<Events::Debug::end>		           / function{Actions::Debug::end}                   = state<States::connected>,
				state<States::debug> + event<Events::Debug::dump_all_registers>    / function{Actions::Debug::dump_all_registers}    = state<States::debug>,
				state<States::debug> + event<Events::Debug::program_all_registers> / function{Actions::Debug::program_all_registers} = state<States::debug>,
				state<States::debug> + event<Events::Debug::ctrlHB_command>        / function{Actions::Debug::ctrlHB_command}        = state<States::debug>,

				state<States::FreqSweep::configured> + event<Events::FreqSweep::end>   / function{Actions::FreqSweep::end}    = state<States::connected>,
				state<States::FreqSweep::configured> + event<Events::FreqSweep::start> / function{Actions::FreqSweep::run}    = state<States::FreqSweep::running>,

				state<States::FreqSweep::running> + event<Events::FreqSweep::Private::complete> = state<States::FreqSweep::configured>,

				*"error_handler"_s + unexpected_event<_> = X
			);
			return ret;
		}
	};

	using T_StateMachine = boost::sml::sm<Connection, boost::sml::process_queue<std::queue>, boost::sml::thread_safe<std::recursive_mutex>>;
}