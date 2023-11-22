#pragma once 

namespace StateMachine {
    template<typename T_bState, typename T_EventData>
    class StateMachine {
    private:
        const T_bState* off_state;
        const T_bState* on_state;
    public:
        T_bState* active_state;
    public:
        constexpr StateMachine (
            T_bState* in_off_state,
            const T_bState* in_on_state
        ) :
            off_state{in_off_state},
            on_state{in_on_state},
            active_state{in_off_state}
        {}
    public:
        inline constexpr bool is_off() const {
            if(active_state == off_state) {
                return true;
            } else {
                return false;
            }
        }

        inline constexpr bool is_on() const {
            return !is_off();
        }

        inline constexpr bool is_active_state_listening_to_events() const {
            if(active_state->jump_info.has_value() == true) {
                return true;
            } else {
                return false;
            }
        }

        inline constexpr bool is_active_state_intermediate_state() const {
            if(active_state->next_state.has_value() == true) {
                return true;
            } else {
                return false;
            }
        }

        constexpr bool turn_on() {
            if(is_off() == true) {
                active_state = const_cast<T_bState*>(static_cast<const T_bState*>(on_state));
                active_state->exec_func_ptr();
                return true;
            } else {
                return false;
            }
        }

        constexpr bool turn_off() {
            if(is_on() == true) {
                active_state = const_cast<T_bState*>(static_cast<const T_bState*>(off_state));
                active_state->exec_func_ptr();
                return true;
            } else {
                return false;
            }
        }

        constexpr bool change_to_state(const T_bState* next_state) {
            if(is_on() == true) {
                active_state = const_cast<T_bState*>(static_cast<const T_bState*>(next_state));
                active_state->exec_func_ptr();
                while(is_active_state_intermediate_state() == true) {
                    active_state = const_cast<T_bState*>(static_cast<const T_bState*>(active_state->next_state.value()));
                    active_state->exec_func_ptr();
                }
                return true;
            } else {
                return false;
            }
        }

        constexpr T_bState* get_next_state_from_event_data(const T_EventData event_data) const {
            for(const auto &i: active_state->jump_info.value()) {
                if(i.has_value() == false) {
                    break;
                }
                if(event_data == (i.value().first)) {
                    return const_cast<T_bState*>(static_cast<const T_bState*>(i.value().second));
                }
            }
            return const_cast<T_bState*>(static_cast<const T_bState*>(active_state->invalid_event_next_state.value()));
        }

        constexpr bool process_event_data(const T_EventData event_data) {
            if(is_active_state_listening_to_events() == true) {
                return change_to_state(get_next_state_from_event_data(event_data));
            } else {
                return false;
            }
        }
    };
}

