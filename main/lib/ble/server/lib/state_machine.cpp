#include <iostream>
#include <span>
#include <array>
#include <cstdint>

#include "ble/server/server.hpp"
#include "trielo/trielo.hpp"
#include "esp_system.h"
#include "util.hpp"

namespace BLE {
	namespace Events {
		namespace Debug {
		}

		namespace FreqSweep {
		};
	}

	namespace Actions {
		void turn_on() {
			std::printf("BLE::StateMachine::Actions::turn_on\n");
			Util::Blinky::get_instance().start(std::chrono::milliseconds(100));
			Server::run();
		};

		void advertise() {
			std::printf("BLE::StateMachine::Actions::advertise\n");
			Util::Blinky::get_instance().set_blink_time(std::chrono::milliseconds(200));
			Server::advertise();
		};

		void connect() {
			std::printf("BLE::StateMachine::Actions::connect\n");
			Util::Blinky::get_instance().set_blink_time(std::chrono::milliseconds(500));
		};

		void disconnect() {
			std::printf("BLE::StateMachine::Actions::disconnect\n");
		};

		void unexpected() {
			std::printf("BLE::StateMachine::Actions::unexpected\n");
			Server::stop();
		}

		void stop() {
			std::printf("BLE::StateMachine::Actions::sleep\n");
			Server::stop();
		}

		void sleep() {
			std::printf("BLE::StateMachine::Actions::sleep\n");
			Util::Blinky::get_instance().stop();
			Server::stop();
		}

		void wakeup() {
			std::printf("BLE::StateMachine::Actions::wakeup\n");
			std::this_thread::sleep_for(std::chrono::milliseconds(5'000));
			Trielo::trielo<esp_restart>();
		}

		namespace Debug {
			static const char *namespace_name = "Debug::";
			void start() {
				std::printf("BLE::StateMachine::Actions::%sstart\n", namespace_name);
			}

			void end() {
				std::printf("BLE::StateMachine::Actions::%send\n", namespace_name);
			}

			void dump(Server::Sender &sender, AD5933::Extension &ad5933) {
				std::unique_lock lock(sender.mutex);
				std::printf("BLE::StateMachine::Actions::%sdump\n", namespace_name);
				const auto ret { ad5933.driver.dump_all_registers() };
				if(ret.has_value()) {
					sender.notify_with_footer<19>(ret.value(), Magic::Packets::Debug::dump_all_registers);
				}
			}

			void program(AD5933::Extension &ad5933, const Events::Debug::program &program_event) {
				std::printf("BLE::StateMachine::Actions::%sprogram\n", namespace_name);
				ad5933.driver.block_write_to_register<12, AD5933::RegAddrs::RW::ControlHB>(program_event.raw_config);
			}

			void command(AD5933::Extension &ad5933, const Events::Debug::command &command_event) {
				std::printf("BLE::StateMachine::Actions::%scommand\n", namespace_name);
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command(command_event.mask));
			}
		}

		namespace FreqSweep {
			static const char *namespace_name = "FreqSweep::";
			//static void configure(bool &processing, AD5933::Extension &ad5933, const Events::FreqSweep::configure &configure_event, boost::sml::back::process<Events::FreqSweep::Private::configure_complete> process_event) {
			void configure(std::atomic<bool> &processing, AD5933::Extension &ad5933, const Events::FreqSweep::configure &configure_event) {
				processing = true;
				std::printf("BLE::StateMachine::Actions::%sconfigure\n", namespace_name);
				ad5933.driver.block_write_to_register<12, AD5933::RegAddrs::RW::ControlHB>(configure_event.raw_config);
				processing = false;
			}

			//static void run(Sender &sender, AD5933::Extension &ad5933, boost::sml::back::process<Events::FreqSweep::Private::sweep_complete> process_event) {
			void run(std::atomic<bool> &processing, Server::Sender &sender, AD5933::Extension &ad5933) {
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

			void end() {
				std::printf("BLE::StateMachine::Actions::%send\n", namespace_name);
			}
		}
	}

	namespace Guards {
		bool processing(std::atomic<bool> &processing) {
			std::printf("Guards::processing: %d\n", processing.load());
			return !processing.load();
		}

		bool neg_processing(std::atomic<bool> &processing) {
			std::printf("Guards::neg_processing: %d\n", processing.load());
			return processing.load();
		}
	}
}