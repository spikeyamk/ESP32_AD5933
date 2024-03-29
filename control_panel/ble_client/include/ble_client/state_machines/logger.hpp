#pragma once

#include <iostream>
#include <boost/sml.hpp>

namespace BLE_Client {
    namespace StateMachines {
        struct Logger {
            static constexpr bool enable { false };
            // https://github.com/boost-ext/sml/blob/master/example/logging.cpp
            template <class SM, class TEvent>
            void log_process_event(const TEvent&) {
                if constexpr(enable) {
                    std::cout
                        << "["
                        << boost::sml::aux::get_type_name<SM>() <<
                        "][process_event] "
                        << boost::sml::aux::get_type_name<TEvent>()
                        << std::endl;
                }
            }

            template <class SM, class TGuard, class TEvent>
            void log_guard(const TGuard&, const TEvent&, bool result) {
                if constexpr(enable) {
                    std::cout
                        << "["
                        << boost::sml::aux::get_type_name<SM>()
                        << "][guard] "
                        << boost::sml::aux::get_type_name<TGuard>()
                        << boost::sml::aux::get_type_name<TEvent>()
                        << (result ? "[OK]" : "[Reject]")
                        << std::endl;
                }
            }

            template <class SM, class TAction, class TEvent>
            void log_action(const TAction&, const TEvent&) {
                if constexpr(enable) {
                    std::cout
                        << "["
                        << boost::sml::aux::get_type_name<SM>()
                        << "][action] "
                        << boost::sml::aux::get_type_name<TAction>()
                        << " "
                        << boost::sml::aux::get_type_name<TEvent>()
                        << std::endl;
                }
            }

            template <class SM, class TSrcState, class TDstState>
            void log_state_change(const TSrcState& src, const TDstState& dst) {
                if constexpr(enable) {
                    std::cout
                        << "["
                        << boost::sml::aux::get_type_name<SM>()
                        << "][transition] "
                        << src.c_str()
                        << " -> "
                        << dst.c_str()
                        << std::endl;
                }
            }
        };
    }
}