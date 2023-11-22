#pragma once

#include "state_machine/state.hpp"
#include "state_machine/state_derived.hpp"
#include "state_machine/state_machine.hpp"
#include "ble_states.hpp"

extern StateMachine::StateMachine<States::bState, decltype(MagicPackets::Debug::Command::start)*> state_machine;
