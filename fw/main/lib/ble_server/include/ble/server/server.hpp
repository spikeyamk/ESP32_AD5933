#pragma once

#include <cstdint>

#include "magic/packets.hpp"

#include <iostream>
#include <queue>
#include <mutex>
#include <functional>
#include <thread>
#include <chrono>
#include <array>
#include <atomic>

#include <boost/sml.hpp>
#include "driver/i2c_master.h"

#include "ad5933/extension/extension.hpp"
#include "ad5933/masks/masks.hpp"

namespace BLE {
    namespace Server {
        void run();
        void inject(const i2c_master_dev_handle_t handle);
        void advertise();
        void stop();
        bool notify(const Magic::Packets::Packet_T &message);
    }

    namespace Server {
        class Sender {
        public:
            std::mutex mutex;
            Sender() = default;

            inline bool notify(const std::array<uint8_t, Magic::Packets::Debug::start.size()> &message) const {
                return BLE::Server::notify(message);
            }

            template<size_t n_bytes>
            inline bool notify_with_footer(const std::array<uint8_t, n_bytes> &message, const Magic::Packets::Packet_T &footer) const {
                static_assert(n_bytes <= Magic::Packets::Debug::start.size(), "n_bytes must be less than or equal to sizeof(Packet_T)");
                auto send_buf = footer;
                std::copy(message.begin(), message.end(), send_buf.begin());
                return BLE::Server::notify(send_buf);
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
		void connect();
		void disconnect();
		void unexpected();
		void sleep();
		void wakeup();
		namespace Debug {
			void start();
			void end();
			void dump(Server::Sender &sender, AD5933::Extension &ad5933);
			void program(AD5933::Extension &ad5933, const Events::Debug::program &program_event);
			void command(AD5933::Extension &ad5933, const Events::Debug::command &command_event);
		}

		namespace FreqSweep {
			//static void configure(bool &processing, AD5933::Extension &ad5933, const Events::FreqSweep::configure &configure_event, boost::sml::back::process<Events::FreqSweep::Private::configure_complete> process_event) {
			void configure(std::atomic<bool> &processing, AD5933::Extension &ad5933, const Events::FreqSweep::configure &configure_event);
			//static void run(Sender &sender, AD5933::Extension &ad5933, boost::sml::back::process<Events::FreqSweep::Private::sweep_complete> process_event) {
			void run(std::atomic<bool> &processing, Server::Sender &sender, AD5933::Extension &ad5933);
			void end();
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
				*state<States::off>        + event<Events::turn_on>      	     / function{Actions::turn_on}      		   = state<States::on>,
				state<States::on>          + event<Events::advertise>													   = state<States::advertise>,
				state<States::advertise>										 / function{Actions::advertise}    		   = state<States::advertising>,
				state<States::advertising> + event<Events::advertise>			 / function{Actions::advertising}    	   = state<States::advertise>,
				state<States::advertising> + event<Events::disconnect>			 / function{Actions::disconnect}    	   = state<States::advertise>,
		       	state<States::advertising> + event<Events::connect>      		 / function{Actions::connect}      		   = state<States::connected>,
		       	state<States::advertising> + event<Events::sleep>      		     / function{Actions::sleep}      		   = state<States::sleeping>,
		       	//state<States::sleeping>    + event<Events::wakeup>      		 / function{Actions::wakeup}      		   = state<States::advertise>,
				state<States::connected>   + event<Events::disconnect>   		 / function{Actions::disconnect}   		   = state<States::advertise>,
				state<States::connected>   + event<Events::Debug::start> 		 / function{Actions::Debug::start} 		   = state<States::debug>,
				state<States::connected>   + event<Events::FreqSweep::configure> / function{Actions::FreqSweep::configure} = state<States::FreqSweep::configuring>,

				state<States::debug> + event<Events::disconnect>     / function{Actions::disconnect}     = state<States::advertise>,
				state<States::debug> + event<Events::Debug::end>     / function{Actions::Debug::end}     = state<States::connected>,
				state<States::debug> + event<Events::Debug::dump>    / function{Actions::Debug::dump}    = state<States::debug>,
				state<States::debug> + event<Events::Debug::program> / function{Actions::Debug::program} = state<States::debug>,
				state<States::debug> + event<Events::Debug::command> / function{Actions::Debug::command} = state<States::debug>,

				state<States::FreqSweep::configuring> + event<Events::disconnect>     									 / function{Actions::disconnect}     = state<States::advertise>,
				state<States::FreqSweep::configuring> + event<Events::FreqSweep::end> 									 / function{Actions::FreqSweep::end} = state<States::connected>,
				state<States::FreqSweep::configuring> + event<Events::FreqSweep::run> [function{Guards::processing}]     / function{Actions::FreqSweep::run} = state<States::FreqSweep::running>,
				state<States::FreqSweep::configuring> + event<Events::FreqSweep::run> [function{Guards::neg_processing}] / function{Actions::unexpected}     = state<States::error>,

				state<States::FreqSweep::running> + event<Events::disconnect>     									       / function{Actions::disconnect}           = state<States::advertise>,
				state<States::FreqSweep::running> + event<Events::FreqSweep::end> 										   / function{Actions::FreqSweep::end}       = state<States::connected>,
				state<States::FreqSweep::running> + event<Events::FreqSweep::run>       [function{Guards::processing}] 	   / function{Actions::FreqSweep::run}       = state<States::FreqSweep::running>,
				state<States::FreqSweep::running> + event<Events::FreqSweep::configure> [function{Guards::processing}] 	   / function{Actions::FreqSweep::configure} = state<States::FreqSweep::configuring>,
				state<States::FreqSweep::running> + event<Events::FreqSweep::configure> [function{Guards::neg_processing}] / function{Actions::unexpected}  		 = state<States::error>,
				state<States::FreqSweep::running> + event<Events::FreqSweep::run>       [function{Guards::neg_processing}] / function{Actions::unexpected} 			 = state<States::error>
			);
			return ret;
		}
	};

	//using T_StateMachine = boost::sml::sm<Connection, boost::sml::process_queue<std::queue>, boost::sml::thread_safe<std::recursive_mutex>>;
	using T_StateMachine = boost::sml::sm<Connection, boost::sml::logger<my_logger>, boost::sml::thread_safe<std::recursive_mutex>>;
}
