#pragma once

#include <iostream>
#include <boost/sml.hpp>

namespace BLE_Client {
    namespace StateMachines {
        struct Logger {
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
    }
}