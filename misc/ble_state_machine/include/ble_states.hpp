#pragma once

#include "state_machine/state.hpp"
#include "state_machine/state_derived.hpp"
#include "magic_packets.hpp"

namespace States {
    const size_t MAX_NUM_OF_JUMPS = 4;
    using FuncPtr = void(*)();
	//using T_Packet = decltype(MagicPackets::Debug::Command::start)*;
    using bState = StateMachine::State::State<FuncPtr, decltype(MagicPackets::Debug::Command::start)*, MAX_NUM_OF_JUMPS>;
    using sState = StateMachine::State::SpecialState<FuncPtr, decltype(MagicPackets::Debug::Command::start)*, MAX_NUM_OF_JUMPS>;
    using iState = StateMachine::State::IntermediateState<FuncPtr, decltype(MagicPackets::Debug::Command::start)*, MAX_NUM_OF_JUMPS>;
    using eState = StateMachine::State::EventListeningState<FuncPtr, decltype(MagicPackets::Debug::Command::start)*, MAX_NUM_OF_JUMPS>;
    //extern std::array<uint8_t, FLASH_PAGE_SIZE> incoming_packet;

	// States before connection with the client is established
	extern const sState off;
	extern const sState on;
	extern const iState advertise;
	extern const sState advertising;
	extern const iState connect;

	// States possible to jump to from the "connected" state
	extern const eState connected;
	extern const iState disconnect;
	extern const iState configure_frequency_sweep;
	extern const eState debug;

	// States related to debug
	extern const iState program_all_registers;
	extern const iState dump_all_registers;
	extern const iState control_HB_command;

	// States related to configuring and initializing the frequency sweep loop
	extern const eState frequency_sweep_configured;
	extern const iState frequency_sweep_cleanup;
	extern const iState initialize_with_start_freq;
	extern const eState with_start_freq_initialized;
	extern const iState start_frequency_sweep;

	// States related to frequency sweep loop
	extern const eState frequency_sweep_loop_top;
	extern const iState check_for_data_valid;
	extern const iState read_data_valid_bool;
	extern const eState data_valid_bool_sent;
	extern const iState read_data_valid_value;
	extern const eState data_valid_value_sent;
	extern const iState repeat_freq;
	extern const iState read_frequency_sweep_complete_bool;
	extern const eState frequency_sweep_complete_bool_sent;
	extern const iState increment_frequency;

	// States to exit out of the frequency sweep loop
	extern const iState repeat_frequency_sweep;
	extern const iState stop_frequency_sweep;
}