#pragma once

#include <iostream>
#include <queue>
#include <mutex>
#include <functional>
#include <thread>
#include <chrono>
#include <span>
#include <atomic>

#include <boost/sml.hpp>

#include "ad5933/extension/extension.hpp"
#include "ad5933/masks/masks.hpp"
#include "ble/server/server.hpp"
#include "ble/sender/sender.hpp"

namespace BLE {
	namespace States {
		struct off {};
		struct on {};
		struct advertise {};
		struct advertising {};
		struct connected {};
		struct error {};

		struct debug;
		namespace FreqSweep {
			struct configure {};
			struct configuring {};
			struct running {};
		}
	}

	namespace Events {
		struct turn_on{};
		struct advertise{};
		struct connect {};

		struct disconnect {};

		namespace Debug {
			struct start {};
			struct end {};
			struct dump {};
			struct program {
				std::span<uint8_t, 12> raw_config;
				program() = delete;
				program(const std::span<uint8_t, 12> &raw_config) :
					raw_config { raw_config }
				{}
			};
			struct command {
				uint8_t mask;
				command() = delete;
				command(const uint8_t command) :
					mask { command }
				{}
            };
		}

		namespace FreqSweep {
			struct configure {
				std::span<uint8_t, 12> raw_config;
				configure() = delete;
				configure(const std::span<uint8_t, 12> &raw_config) :
					raw_config { raw_config }
				{}
			};
			struct start {};
			struct end {};
			namespace Private {
				struct configure_complete {}; //Commented out because I want to make it inaccessible to other translation units
				struct sweep_complete {}; //Commented out because I want to make it inaccessible to other translation units
			}
		};
	}

	namespace Actions {
		static void turn_on() {
			std::printf("BLE::StateMachine::Actions::turn_on\n");
			Server::run();
		};

		static void advertise() {
			std::printf("BLE::StateMachine::Actions::advertise\n");
			Server::advertise();
		};

		static void connect() {
			std::printf("BLE::StateMachine::Actions::connect\n");
		};

		static void disconnect() {
			std::printf("BLE::StateMachine::Actions::disconnect\n");
		};

		static void unexpected() {
			std::printf("BLE::StateMachine::Actions::unexpected\n");
			Server::stop();
		}

		namespace Debug {
			static const char *namespace_name = "Debug::";
			static void start() {
				std::printf("BLE::StateMachine::Actions::%sdisconnect\n", namespace_name);
			}

			static void end() {
				std::printf("BLE::StateMachine::Actions::%send\n", namespace_name);
			}

			static void dump(Sender &sender, AD5933::Extension &ad5933) {
				std::unique_lock lock(sender.mutex);
				std::printf("BLE::StateMachine::Actions::%sdump\n", namespace_name);
				const auto ret { ad5933.driver.dump_all_registers() };
				if(ret.has_value()) {
					sender.notify_with_footer<19>(ret.value(), Magic::Packets::Debug::dump_all_registers);
				}
			}

			static void program(AD5933::Extension &ad5933, const Events::Debug::program &program_event) {
				std::printf("BLE::StateMachine::Actions::%sprogram\n", namespace_name);
				ad5933.driver.block_write_to_register<12, AD5933::RegAddrs::RW::ControlHB>(program_event.raw_config);
			}

			static void command(AD5933::Extension &ad5933, const Events::Debug::command &command_event) {
				std::printf("BLE::StateMachine::Actions::%scommand\n", namespace_name);
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command(command_event.mask));
			}
		}

		namespace FreqSweep {
			static const char *namespace_name = "FreqSweep::";
			//static void configure(bool &processing, AD5933::Extension &ad5933, const Events::FreqSweep::configure &configure_event, boost::sml::back::process<Events::FreqSweep::Private::configure_complete> process_event) {
			static void configure(std::atomic<bool> &processing, AD5933::Extension &ad5933, const Events::FreqSweep::configure &configure_event) {
				processing = true;
				std::printf("BLE::StateMachine::Actions::%sconfigure\n", namespace_name);
				ad5933.driver.block_write_to_register<12, AD5933::RegAddrs::RW::ControlHB>(configure_event.raw_config);
				processing = false;
			}

			//static void run(Sender &sender, AD5933::Extension &ad5933, boost::sml::back::process<Events::FreqSweep::Private::sweep_complete> process_event) {
			static void run(std::atomic<bool> &processing, Sender &sender, AD5933::Extension &ad5933) {
				std::unique_lock lock(sender.mutex);
				processing = true;
				std::printf("BLE::StateMachine::Actions::%srun\n", namespace_name);
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::StandbyMode);
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::InitStartFreq);
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::StartFreqSweep);
				do {
					while(ad5933.has_status_condition(AD5933::Masks::Or::Status::ValidData) == false) {
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
					}
					const auto data { ad5933.read_impe_data() };
					if(data.has_value()) {
						sender.notify_with_footer<4>(data.value(), Magic::Packets::FrequencySweep::read_data_valid_value);
					}
					ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::IncFreq);
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				} while(ad5933.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete) == false);

				std::this_thread::sleep_for(std::chrono::seconds(1));
				processing = false;
			}

			static void end() {
				std::printf("BLE::StateMachine::Actions::%send\n", namespace_name);
			}
		}
	}

	namespace Guards {
		static bool processing(std::atomic<bool> &processing) {
			std::printf("Guards::processing: %d\n", processing.load());
			return !processing.load();
		}

		static bool neg_processing(std::atomic<bool> &processing) {
			std::printf("Guards::neg_processing: %d\n", processing.load());
			return processing.load();
		}
	}

	struct Connection {
		auto operator()() const {
			using namespace boost::sml;
			using namespace std;
			auto ret = make_transition_table(
				//state<States::off>        + event<Events::turn_on>      	     / function{Actions::turn_on}      		   = state<States::on>,
				//state<States::on>          + event<Events::advertise>    		 / function{Actions::advertise}    		   = state<States::advertising>,
				//state<States::advertise>										 / function{Actions::advertise}    		   = state<States::advertising>,
		       *state<States::advertising> + event<Events::connect>      		 / function{Actions::connect}      		   = state<States::connected>,
				state<States::connected>   + event<Events::disconnect>   		 / function{Actions::disconnect}   		   = state<States::advertising>,
				state<States::connected>   + event<Events::Debug::start> 		 / function{Actions::Debug::start} 		   = state<States::debug>,
				state<States::connected>   + event<Events::FreqSweep::configure> / function{Actions::FreqSweep::configure} = state<States::FreqSweep::configuring>,

				state<States::debug> + event<Events::disconnect>     / function{Actions::disconnect}     = state<States::advertising>,
				state<States::debug> + event<Events::Debug::end>     / function{Actions::Debug::end}     = state<States::connected>,
				state<States::debug> + event<Events::Debug::dump>    / function{Actions::Debug::dump}    = state<States::debug>,
				state<States::debug> + event<Events::Debug::program> / function{Actions::Debug::program} = state<States::debug>,
				state<States::debug> + event<Events::Debug::command> / function{Actions::Debug::command} = state<States::debug>,

				state<States::FreqSweep::configuring> + event<Events::disconnect>     									   / function{Actions::disconnect}     = state<States::advertising>,
				state<States::FreqSweep::configuring> + event<Events::FreqSweep::end> 									   / function{Actions::FreqSweep::end} = state<States::connected>,
				state<States::FreqSweep::configuring> + event<Events::FreqSweep::start> [function{Guards::processing}]     / function{Actions::FreqSweep::run} = state<States::FreqSweep::running>,
				state<States::FreqSweep::configuring> + event<Events::FreqSweep::start> [function{Guards::neg_processing}] / function{Actions::unexpected}     = state<States::error>,

				state<States::FreqSweep::running> + event<Events::disconnect>     									       / function{Actions::disconnect}           = state<States::advertising>,
				state<States::FreqSweep::running> + event<Events::FreqSweep::end> 										   / function{Actions::FreqSweep::end}       = state<States::connected>,
				state<States::FreqSweep::running> + event<Events::FreqSweep::start>     [function{Guards::processing}] 	   / function{Actions::FreqSweep::run}       = state<States::FreqSweep::running>,
				state<States::FreqSweep::running> + event<Events::FreqSweep::configure> [function{Guards::processing}] 	   / function{Actions::FreqSweep::configure} = state<States::FreqSweep::configuring>,
				state<States::FreqSweep::running> + event<Events::FreqSweep::configure> [function{Guards::neg_processing}] / function{Actions::unexpected}  		 = state<States::error>,
				state<States::FreqSweep::running> + event<Events::FreqSweep::start>     [function{Guards::neg_processing}] / function{Actions::unexpected} 			 = state<States::error>
			);
			return ret;
		}
	};

	//using T_StateMachine = boost::sml::sm<Connection, boost::sml::process_queue<std::queue>, boost::sml::thread_safe<std::recursive_mutex>>;
	using T_StateMachine = boost::sml::sm<Connection, boost::sml::thread_safe<std::recursive_mutex>>;
}