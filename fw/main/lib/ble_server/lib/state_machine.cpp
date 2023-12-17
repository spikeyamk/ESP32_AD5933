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
			Util::Blinky::get_instance().start(std::chrono::milliseconds(100));
			Server::run();
		};

		void advertise() {
			Util::Blinky::get_instance().set_blink_time(std::chrono::milliseconds(200));
			Server::advertise();
		};

		void advertising() {
		};

		void connect() {
			Util::Blinky::get_instance().set_blink_time(std::chrono::milliseconds(500));
		};

		void disconnect() {

		};

		void unexpected() {
			Util::Blinky::get_instance().stop();
			Server::stop();
		}

		void sleep() {
			Util::Blinky::get_instance().stop();
			Server::stop();
		}

		namespace Debug {
			static const char *namespace_name = "Debug::";
			void start() {
			}

			void end() {
			}

			void dump(Server::Sender &sender, AD5933::Extension &ad5933) {
				std::unique_lock lock(sender.mutex);
				const auto ret { ad5933.driver.dump_all_registers() };
				if(ret.has_value()) {
					sender.notify_with_footer<19>(ret.value(), Magic::Packets::Debug::dump_all_registers);
				}
			}

			void program(AD5933::Extension &ad5933, const Events::Debug::program &program_event) {
				std::for_each(program_event.raw_config.end(), program_event.raw_config.end(), [index = 0](const auto e) mutable {
					if(index % 8 == 0){
						std::printf("\n");
					}
					std::printf("0x%02X, ", e);
					index++;
				});
				std::printf("\n");

				ad5933.driver.block_write_to_register<12, AD5933::RegAddrs::RW::ControlHB>(program_event.raw_config);
			}

			void command(AD5933::Extension &ad5933, const Events::Debug::command &command_event) {
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command(command_event.mask));
			}
		}

		namespace FreqSweep {
			static const char *namespace_name = "FreqSweep::";
			//static void configure(bool &processing, AD5933::Extension &ad5933, const Events::FreqSweep::configure &configure_event, boost::sml::back::process<Events::FreqSweep::Private::configure_complete> process_event) {
			void configure(std::atomic<bool> &processing, AD5933::Extension &ad5933, const Events::FreqSweep::configure &configure_event) {
				processing = true;
				std::for_each(configure_event.raw_config.end(), configure_event.raw_config.end(), [index = 0](const auto e) mutable {
					if(index % 8 == 0){
						std::printf("\n");
					}
					std::printf("0x%02X, ", e);
					index++;
				});
				std::printf("\n");
				ad5933.driver.block_write_to_register<12, AD5933::RegAddrs::RW::ControlHB>(configure_event.raw_config);
				processing = false;
			}

			//static void run(Sender &sender, AD5933::Extension &ad5933, boost::sml::back::process<Events::FreqSweep::Private::sweep_complete> process_event) {
			void run(std::atomic<bool> &processing, Server::Sender &sender, AD5933::Extension &ad5933) {
				std::unique_lock lock(sender.mutex);
				processing = true;
				ad5933.reset();
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::StandbyMode);
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::InitStartFreq);
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::StartFreqSweep);
				do {
					while(ad5933.has_status_condition(AD5933::Masks::Or::Status::ValidData) == false) {
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
					const auto data { ad5933.read_impe_data() };
					if(data.has_value()) {
						sender.notify_with_footer<4>(data.value(), Magic::Packets::FrequencySweep::read_data_valid_value);
					}
					ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::IncFreq);
				} while(ad5933.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete) == false);
				
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				processing = false;
			}

			void end() {
			}
		}
	}

	namespace Guards {
		bool processing(std::atomic<bool> &processing) {
			return !processing.load();
		}

		bool neg_processing(std::atomic<bool> &processing) {
			return processing.load();
		}
	}
}