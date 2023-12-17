#pragma once

#include <cstddef>

namespace StateMachine {
	namespace State {
		template<typename T_FuncPtr, typename T_EventData, size_t MaxNumOfJumps>
		class SpecialState : public State<T_FuncPtr, T_EventData, MaxNumOfJumps> {
			using Base = State<T_FuncPtr, T_EventData, MaxNumOfJumps>;
			using T_JumpPair = std::pair<T_EventData, const Base*>;
			using T_JumpPairOptional = std::optional<T_JumpPair>;
			using T_JumpPairArray = std::array<T_JumpPairOptional, MaxNumOfJumps>;
			using T_JumpInfo = T_JumpPairArray;
		public:
			constexpr SpecialState(
				//const T_FuncPtr in_func_ptr
			) :
				Base {
					//in_func_ptr
				}
			{}

			constexpr SpecialState(
				const T_FuncPtr in_func_ptr
			) :
				Base {
					in_func_ptr
				}
			{}
		};

		template<typename T_FuncPtr, typename T_EventData, size_t MaxNumOfJumps>
		class IntermediateState : public State<T_FuncPtr, T_EventData, MaxNumOfJumps> {
			using Base = State<T_FuncPtr, T_EventData, MaxNumOfJumps>;
			using T_JumpPair = std::pair<T_EventData, const Base*>;
			using T_JumpPairOptional = std::optional<T_JumpPair>;
			using T_JumpPairArray = std::array<T_JumpPairOptional, MaxNumOfJumps>;
			using T_JumpInfo = T_JumpPairArray;
		public:
			constexpr IntermediateState(
				const T_FuncPtr in_func_ptr,
				const Base* in_next_state
			) :
				Base {
					in_func_ptr,
					in_next_state
				}
			{}
		};

		template<typename T_FuncPtr, typename T_EventData, size_t MaxNumOfJumps>
		class EventListeningState : public State<T_FuncPtr, T_EventData, MaxNumOfJumps> {
			using Base = State<T_FuncPtr, T_EventData, MaxNumOfJumps>;
			using T_JumpPair = std::pair<T_EventData, const Base*>;
			using T_JumpPairOptional = std::optional<T_JumpPair>;
			using T_JumpPairArray = std::array<T_JumpPairOptional, MaxNumOfJumps>;
			using T_JumpInfo = T_JumpPairArray;
		public:
			constexpr EventListeningState (
				//const T_FuncPtr in_func_ptr,
				const T_JumpInfo &in_jump_info,
				const Base* in_invalid_next_state
			) :
				Base {
					//in_func_ptr,
					in_jump_info,
					in_invalid_next_state
				}
			{}

			constexpr EventListeningState (
				const T_FuncPtr in_func_ptr,
				const T_JumpInfo &in_jump_info,
				const Base* in_invalid_next_state
			) :
				Base {
					in_func_ptr,
					in_jump_info,
					in_invalid_next_state
				}
			{}
		};
	}
};

