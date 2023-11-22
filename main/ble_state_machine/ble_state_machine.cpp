#include "include/ble_state_machine.hpp"

constinit StateMachine::StateMachine<States::bState, decltype(MagicPackets::Debug::Command::start)*> state_machine {
	const_cast<States::bState*>(static_cast<const States::bState*>(&States::off)),
	const_cast<States::bState*>(static_cast<const States::bState*>(&States::on))
};