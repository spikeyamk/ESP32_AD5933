#include <mutex>
#include <cassert>

#include <boost/sml.hpp>

#include "ble/state_machine/state_machine.hpp"
#include "ble/state_machine/sender.hpp"

#include "ble/state_machine/tests.hpp"

namespace BLE {
    namespace StateMachine_Tests {
        void fun_foo(T_StateMachine &sm) {
            sm.process_event(Events::turn_on{});
        }

        int run() {
            namespace sml = boost::sml;
            Sender sender{};
            T_StateMachine nimble_sm{sender};
            {
                assert(nimble_sm.is(sml::state<States::off>));

                //nimble_sm.process_event(Events::turn_on{});
                fun_foo(nimble_sm);
                assert(nimble_sm.is(sml::state<States::advertising>));

                nimble_sm.process_event(Events::connect{});
                assert(nimble_sm.is(sml::state<States::connected>));

                nimble_sm.process_event(Events::disconnect{});
                assert(nimble_sm.is(sml::state<States::advertising>));

                nimble_sm.process_event(Events::connect{});
                assert(nimble_sm.is(sml::state<States::connected>));
            }

            {
                nimble_sm.process_event(Events::Debug::start{});
                assert(nimble_sm.is(sml::state<States::debug>));

                nimble_sm.process_event(Events::Debug::dump_all_registers{});
                assert(nimble_sm.is(sml::state<States::debug>));

                nimble_sm.process_event(Events::Debug::program_all_registers{});
                assert(nimble_sm.is(sml::state<States::debug>));

                nimble_sm.process_event(Events::Debug::ctrlHB_command{ 0xFF });
                assert(nimble_sm.is(sml::state<States::debug>));

                nimble_sm.process_event(Events::Debug::end{});
                assert(nimble_sm.is(sml::state<States::connected>));
            }

            {
                nimble_sm.process_event(Events::FreqSweep::configure{});
                assert(nimble_sm.is(sml::state<States::FreqSweep::configured>));

                std::thread([&nimble_sm]() { nimble_sm.process_event(Events::FreqSweep::start{}); }).detach();
                while(nimble_sm.is(sml::state<States::FreqSweep::running>) == false) {
                    std::this_thread::yield();
                }
                assert(nimble_sm.is(sml::state<States::FreqSweep::running>));

                while(nimble_sm.is(sml::state<States::FreqSweep::running>) == true) {
                    std::unique_lock lock(sender.mutex);
                    sender.send("hahaha");
                }

                nimble_sm.process_event(Events::FreqSweep::end{});
                assert(nimble_sm.is(sml::state<States::connected>));
            }

            return 0;
        }
    } 
}