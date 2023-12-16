#include <mutex>
#include <cassert>

#include <boost/sml.hpp>

#include "state_machine.hpp"
#include "sender.hpp"

#include "foo.hpp"

void fun_foo(NimBLE::T_StateMachine &sm) {
    sm.process_event(NimBLE::Events::turn_on{});
}

void run_foo() {
	namespace sml = boost::sml;
	NimBLE::Sender sender{};
	NimBLE::T_StateMachine nimble_sm{sender};
	{
		assert(nimble_sm.is(sml::state<NimBLE::States::off>));

		//nimble_sm.process_event(NimBLE::Events::turn_on{});
		fun_foo(nimble_sm);
		assert(nimble_sm.is(sml::state<NimBLE::States::advertising>));

		nimble_sm.process_event(NimBLE::Events::connect{});
		assert(nimble_sm.is(sml::state<NimBLE::States::connected>));

		nimble_sm.process_event(NimBLE::Events::disconnect{});
		assert(nimble_sm.is(sml::state<NimBLE::States::advertising>));

		nimble_sm.process_event(NimBLE::Events::connect{});
		assert(nimble_sm.is(sml::state<NimBLE::States::connected>));
	}

	{
		nimble_sm.process_event(NimBLE::Events::Debug::start{});
		assert(nimble_sm.is(sml::state<NimBLE::States::debug>));

		nimble_sm.process_event(NimBLE::Events::Debug::dump_all_registers{});
		assert(nimble_sm.is(sml::state<NimBLE::States::debug>));

		nimble_sm.process_event(NimBLE::Events::Debug::program_all_registers{});
		assert(nimble_sm.is(sml::state<NimBLE::States::debug>));

		nimble_sm.process_event(NimBLE::Events::Debug::command{});
		assert(nimble_sm.is(sml::state<NimBLE::States::debug>));

		nimble_sm.process_event(NimBLE::Events::Debug::end{});
		assert(nimble_sm.is(sml::state<NimBLE::States::connected>));
	}

	{
		nimble_sm.process_event(NimBLE::Events::FreqSweep::configure{});
		assert(nimble_sm.is(sml::state<NimBLE::States::FreqSweep::configured>));

		std::thread([&nimble_sm]() { nimble_sm.process_event(NimBLE::Events::FreqSweep::start{}); }).detach();
		while(nimble_sm.is(sml::state<NimBLE::States::FreqSweep::running>) == false) {
			std::this_thread::yield();
		}
		assert(nimble_sm.is(sml::state<NimBLE::States::FreqSweep::running>));

		while(nimble_sm.is(sml::state<NimBLE::States::FreqSweep::running>) == true) {
			std::unique_lock lock(sender.mutex);
			sender.send("hahaha");
		}

		nimble_sm.process_event(NimBLE::Events::FreqSweep::end{});
		assert(nimble_sm.is(sml::state<NimBLE::States::connected>));
	}
}
