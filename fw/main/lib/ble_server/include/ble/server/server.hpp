#pragma once

#include <cstdint>
#include <iostream>
#include <queue>
#include <mutex>
#include <functional>
#include <thread>
#include <chrono>
#include <array>
#include <atomic>
#include <span>
#include <memory>

#include <boost/sml.hpp>
#include "driver/i2c_master.h"

#include "ad5933/extension/extension.hpp"
#include "ad5933/masks/masks.hpp"
#include "magic/events/commands.hpp"
#include "magic/packets/outcoming.hpp"

namespace BLE {
    namespace Server {
        void run();
        void inject(const i2c_master_dev_handle_t handle);
        void advertise();
        void stop();
		bool notify_hid_information(const std::span<uint8_t, std::dynamic_extent>& message);
        bool notify_body_composition_measurement(const std::span<uint8_t, std::dynamic_extent>& message);
    }

	struct StopSources {
		bool save { false };
		bool send { false };
	};

    namespace Server {
        class Sender {
        public:
            std::mutex mutex;
            Sender() = default;

			template<typename T_OutComingPacket>
			inline bool notify_hid_information(const T_OutComingPacket& event) const {
				auto tmp_array { event.get_raw_data() };
				return BLE::Server::notify_hid_information(std::span(tmp_array.begin(), tmp_array.end()));
            }

			template<typename T_OutComingPacket>
			inline bool notify_body_composition_measurement(const T_OutComingPacket& event) const {
				auto tmp_array { event.get_raw_data() };
				return BLE::Server::notify_body_composition_measurement(std::span(tmp_array.begin(), tmp_array.end()));
            }
        };
    }

	namespace States {
		struct off {};
		struct on {};
		struct advertise {};
		struct advertising {};
		struct connected {};
		struct sleeping {};
		struct error {};

		struct debug;
		namespace FreqSweep {
			struct configure {};
			struct configuring {};
			struct running {};
		}
		struct file {};
		namespace Auto {
			struct save {};
			struct send {};
		}
	}

	namespace Events {
		struct turn_on{};
		struct advertise{};
		struct connect {};

		struct disconnect {};
		struct sleep {};
		struct wakeup {};

		namespace Debug {
			struct start {};
			struct end {};
			struct dump {};
			struct program {
				std::array<uint8_t, 12> raw_config {};
			};
			struct command {
				uint8_t mask = 0;
            };
		}

		namespace FreqSweep {
			struct configure {
				std::array<uint8_t, 12> raw_config {};
			};
			struct run {};
			struct end {};
			namespace Private {
				struct configure_complete {}; //Commented out because I want to make it inaccessible to other translation units
				struct sweep_complete {}; //Commented out because I want to make it inaccessible to other translation units
			}
		};
	}

	namespace Actions {
		void turn_on();
		void advertise();
		void advertising();
		void set_connected_blink_time();
		void disconnect();
		void unexpected();
		void sleep();
		void wakeup();
		namespace Debug {
			void start();
			void end();
			void dump(std::shared_ptr<Server::Sender> &sender, AD5933::Extension &ad5933);
			void program(AD5933::Extension &ad5933, const Magic::Events::Commands::Debug::Program& program_event);
			void command(AD5933::Extension &ad5933, const Magic::Events::Commands::Debug::CtrlHB &ctrl_HB_event);
		}

		namespace FreqSweep {
			//static void configure(bool &processing, AD5933::Extension &ad5933, const Events::FreqSweep::configure &configure_event, boost::sml::back::process<Events::FreqSweep::Private::configure_complete> process_event) {
			void configure(std::atomic<bool> &processing, AD5933::Extension &ad5933, const Magic::Events::Commands::Sweep::Configure &configure_event);
			//static void run(Sender &sender, AD5933::Extension &ad5933, boost::sml::back::process<Events::FreqSweep::Private::sweep_complete> process_event) {
			void run(std::atomic<bool> &processing, std::shared_ptr<Server::Sender>& sender, AD5933::Extension &ad5933);
			void end();
		}

		namespace File {
			void free(std::shared_ptr<Server::Sender>& sender);
			void list_count(std::shared_ptr<Server::Sender>& sender);
			void list(std::shared_ptr<Server::Sender>& sender);
			void size(const Magic::Events::Commands::File::Size& event, std::shared_ptr<Server::Sender>& sender);
			void remove(const Magic::Events::Commands::File::Remove& event);
			void download(const Magic::Events::Commands::File::Download& event, std::shared_ptr<Server::Sender>& sender);
			void upload(const Magic::Events::Commands::File::Upload& event);
		}

		namespace Auto {
			void start_saving(std::shared_ptr<Server::Sender>& sender, StopSources& stop_sources, AD5933::Extension &ad5933);
			void start_sending(std::shared_ptr<Server::Sender>& sender, StopSources& stop_sources, AD5933::Extension &ad5933);
			void stop_saving(StopSources& stop_sources);
			void stop_sending(StopSources& stop_sources);
		}
	}

	namespace Guards {
		bool processing(std::atomic<bool> &processing);
		bool neg_processing(std::atomic<bool> &processing);
	}

	struct my_logger {
		// https://github.com/boost-ext/sml/blob/master/example/logging.cpp
		template <class SM, class TEvent>
		void log_process_event(const TEvent&) {
			std::printf("[%s][process_event] %s\n",
				boost::sml::aux::get_type_name<SM>(),
				boost::sml::aux::get_type_name<TEvent>()
			);
		}

		template <class SM, class TGuard, class TEvent>
		void log_guard(const TGuard&, const TEvent&, bool result) {
			std::printf("[%s][guard] %s %s %s\n",
				boost::sml::aux::get_type_name<SM>(),
				boost::sml::aux::get_type_name<TGuard>(),
				boost::sml::aux::get_type_name<TEvent>(), (result ? "[OK]" : "[Reject]")
			);
		}

		template <class SM, class TAction, class TEvent>
		void log_action(const TAction&, const TEvent&) {
			std::printf("[%s][action] %s %s\n",
				boost::sml::aux::get_type_name<SM>(),
				boost::sml::aux::get_type_name<TAction>(),
				boost::sml::aux::get_type_name<TEvent>()
			);
		}

		template <class SM, class TSrcState, class TDstState>
		void log_state_change(const TSrcState& src, const TDstState& dst) {
			std::printf("[%s][transition] %s -> %s\n",
				boost::sml::aux::get_type_name<SM>(),
				src.c_str(),
				dst.c_str()
			);
		}
	};

	struct Connection {
		auto operator()() const {
			using namespace boost::sml;
			using namespace std;
			auto ret = make_transition_table(
				*state<States::off>        + event<Events::turn_on>      	     / function{Actions::turn_on}      		       = state<States::on>,
				state<States::on>          + event<Events::advertise>													       = state<States::advertise>,

				state<States::advertise>										 / function{Actions::advertise}    		       = state<States::advertising>,
				state<States::advertising> + event<Events::advertise>			 / function{Actions::advertising}    	       = state<States::advertise>,
				state<States::advertising> + event<Events::disconnect>			 / function{Actions::disconnect}    	       = state<States::advertise>,
		       	state<States::advertising> + event<Events::connect>      		 / function{Actions::set_connected_blink_time} = state<States::connected>,
		       	state<States::advertising> + event<Events::sleep>      		     / function{Actions::sleep}					   = state<States::sleeping>,

		       	//state<States::sleeping>    + event<Events::wakeup>      		 / function{Actions::wakeup}      		   = state<States::advertise>,

				state<States::connected>   + event<Events::disconnect>   		 / function{Actions::disconnect}   		   	   = state<States::advertise>,
				state<States::connected>   + event<Magic::Events::Commands::Debug::Start> / function{Actions::Debug::start}    = state<States::debug>,
				state<States::connected>   + event<Magic::Events::Commands::Sweep::Configure> / function{Actions::FreqSweep::configure} = state<States::FreqSweep::configuring>,
				state<States::connected>   + event<Magic::Events::Commands::File::Start>									   = state<States::file>,
				state<States::connected>   + event<Magic::Events::Commands::Auto::Save>	/ function{Actions::Auto::start_saving}  = state<States::Auto::save>,
				state<States::connected>   + event<Magic::Events::Commands::Auto::Send>	/ function{Actions::Auto::start_sending} = state<States::Auto::send>,

				state<States::debug> + event<Events::disconnect>     / function{Actions::disconnect}     = state<States::advertise>,
				state<States::debug> + event<Events::Debug::end>     / function{Actions::Debug::end}     = state<States::connected>,
				state<States::debug> + event<Magic::Events::Commands::Debug::Dump> / function{Actions::Debug::dump}    = state<States::debug>,
				state<States::debug> + event<Magic::Events::Commands::Debug::Program> / function{Actions::Debug::program} = state<States::debug>,
				state<States::debug> + event<Magic::Events::Commands::Debug::CtrlHB> / function{Actions::Debug::command} = state<States::debug>,

				state<States::FreqSweep::configuring> + event<Events::disconnect>     									 / function{Actions::disconnect}     = state<States::advertise>,
				state<States::FreqSweep::configuring> + event<Magic::Events::Commands::Sweep::End> 						 / function{Actions::FreqSweep::end} = state<States::connected>,
				state<States::FreqSweep::configuring> + event<Magic::Events::Commands::Sweep::Run> [function{Guards::processing}]     / function{Actions::FreqSweep::run} = state<States::FreqSweep::running>,
				state<States::FreqSweep::configuring> + event<Magic::Events::Commands::Sweep::Run> [function{Guards::neg_processing}] / function{Actions::unexpected}     = state<States::error>,

				state<States::FreqSweep::running> + event<Events::disconnect>     									       / function{Actions::disconnect}           				 = state<States::advertise>,
				state<States::FreqSweep::running> + event<Magic::Events::Commands::Sweep::End> 							   / function{Actions::FreqSweep::end}       				 = state<States::connected>,
				state<States::FreqSweep::running> + event<Magic::Events::Commands::Sweep::Run>       [function{Guards::processing}] 	   / function{Actions::FreqSweep::run}       = state<States::FreqSweep::running>,
				state<States::FreqSweep::running> + event<Magic::Events::Commands::Sweep::Configure> [function{Guards::processing}] 	   / function{Actions::FreqSweep::configure} = state<States::FreqSweep::configuring>,
				state<States::FreqSweep::running> + event<Magic::Events::Commands::Sweep::Configure> [function{Guards::neg_processing}] / function{Actions::unexpected} 			 = state<States::error>,
				state<States::FreqSweep::running> + event<Magic::Events::Commands::Sweep::Run>       [function{Guards::neg_processing}] / function{Actions::unexpected} 			 = state<States::error>,

				// for file commands
				state<States::file> + event<Events::disconnect> / function{Actions::disconnect} = state<States::advertise>,
				state<States::file> + event<Magic::Events::Commands::File::End> = state<States::connected>,
				state<States::file> + event<Magic::Events::Commands::File::Free> / function{Actions::File::free} = state<States::file>,
				state<States::file> + event<Magic::Events::Commands::File::ListCount> / function{Actions::File::list_count} = state<States::file>,
				state<States::file> + event<Magic::Events::Commands::File::List> / function{Actions::File::list} = state<States::file>,
				state<States::file> + event<Magic::Events::Commands::File::Size> / function{Actions::File::size} = state<States::file>,
				state<States::file> + event<Magic::Events::Commands::File::Remove> / function{Actions::File::remove} = state<States::file>,
				state<States::file> + event<Magic::Events::Commands::File::Download> / function{Actions::File::download} = state<States::file>,
				state<States::file> + event<Magic::Events::Commands::File::Upload> / function{Actions::File::upload} = state<States::file>,

				// for auto save commands
				state<States::Auto::save>  + event<Events::disconnect> / function{Actions::Auto::stop_saving} = state<States::advertise>,
				state<States::Auto::save>  + event<Magic::Events::Commands::Auto::End> / function{Actions::Auto::stop_saving} = state<States::connected>,

				// for auto send commands
				state<States::Auto::send>  + event<Events::disconnect> / function{Actions::Auto::stop_sending} = state<States::advertise>,
				state<States::Auto::send>  + event<Magic::Events::Commands::Auto::End> / function{Actions::Auto::stop_sending} = state<States::connected>
			);
			return ret;
		}
	};

	//using T_StateMachine = boost::sml::sm<Connection, boost::sml::process_queue<std::queue>, boost::sml::thread_safe<std::recursive_mutex>>;
	using T_StateMachine = boost::sml::sm<Connection, boost::sml::logger<my_logger>, boost::sml::thread_safe<std::recursive_mutex>>;
}
