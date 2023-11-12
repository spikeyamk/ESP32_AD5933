#pragma once

#include <optional>
#include <functional>
#include <cstdint>
#include <cstddef>

namespace StateMachine {
	namespace State {
		template<typename T_FuncPtr, typename T_EventData, size_t MaxNumOfJumps>
		class State {
			using Base = State<T_FuncPtr, T_EventData, MaxNumOfJumps>;
			using T_JumpPair = std::pair<T_EventData, const Base*>;
			using T_JumpPairOptional = std::optional<T_JumpPair>;
			using T_JumpPairArray = std::array<T_JumpPairOptional, MaxNumOfJumps>;
			using T_JumpInfo = T_JumpPairArray;
		public:
			std::optional<T_FuncPtr> func_ptr {std::nullopt};

			/* Special member variable for intermediate state */
			std::optional<const Base*> next_state {std::nullopt};

			/* Special member variables for event listening state */
			std::optional<T_JumpInfo>jump_info {std::nullopt};
			std::optional<const Base*> invalid_event_next_state {std::nullopt};
		public:
			// These constructors are for special state
			constexpr State(
				//const T_FuncPtr in_func_ptr
			)
				//func_ptr{in_func_ptr}
			{}
			constexpr State(
				const T_FuncPtr in_func_ptr
			) :
				func_ptr{in_func_ptr}
			{}
		protected:
			// This constructor is for intermediate state
			constexpr State(
				const T_FuncPtr in_func_ptr,
				const Base* in_next_state
			) :
				func_ptr{in_func_ptr},
				next_state{in_next_state}
			{}
		protected:
			// These constructors are for event listening state
			constexpr State(
				//const T_FuncPtr in_func_ptr,
				const T_JumpInfo &in_jump_info,
				const Base* in_invalid_next_state
			) :
				//func_ptr{in_func_ptr},
				jump_info{in_jump_info},
				invalid_event_next_state{in_invalid_next_state}
			{}
			constexpr State(
				const T_FuncPtr in_func_ptr,
				const T_JumpInfo &in_jump_info,
				const Base* in_invalid_next_state
			) :
				func_ptr{in_func_ptr},
				jump_info{in_jump_info},
				invalid_event_next_state{in_invalid_next_state}
			{}
		public:
			/*
			constexpr bool operator==(const Base &rhs) const {
				if(unique_state_id == rhs.unique_state_id
				&& func_ptr == rhs.func_ptr
				&& jump_info == rhs.jump_info
				&& next_state == rhs.next_state
				&& invalid_event_next_state == rhs.invalid_event_next_state
				) {
					return true;
				} else {
					return false;
				}
			}
			*/

			constexpr bool exec_func_ptr() const {
				if(func_ptr.has_value() == true) {
					std::invoke(func_ptr.value());
					return true;
				} else {
					return false;
				}
			}
		};
	}
}



