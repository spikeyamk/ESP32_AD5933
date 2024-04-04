#pragma once

#include <algorithm>
#include <limits>

#include "serde/common.hpp"

namespace Serde {
	template<typename ... Args>
	class Serializer {
	private:
		template<typename T, const size_t obj_size, const size_t obj_i, const size_t ser_size, const size_t ser_i>
		requires std::is_scalar_v<std::remove_reference_t<decltype(boost::pfr::get<obj_i>(T{}))>>
		static void p_inner_serialize(const T& obj, std::array<uint8_t, ser_size>& ret) {
			constexpr size_t ser_inc { sizeof(std::remove_reference_t<decltype(boost::pfr::get<obj_i>(obj))>) };

			std::generate(
				ret.begin() + ser_i,
				ret.begin() + ser_i + ser_inc,
				[
					index = static_cast<size_t>(0),
					obj_mem = static_cast<const int64_t*>(
						static_cast<const void*>(
							&boost::pfr::get<obj_i>(obj)
						)
					)
				]() mutable {
					return static_cast<uint8_t>(
						(*obj_mem >> (index++ * 8))
						& 0xFF
					);
				}
			);

			if constexpr(obj_i + 1 < obj_size) {
				p_inner_serialize<T, obj_size, obj_i + 1, ser_size, ser_i + ser_inc>(obj, ret);
			}
		}

		template<typename T, const size_t obj_size, const size_t obj_i, const size_t ser_size, const size_t ser_i>
		requires is_iterable_v<std::remove_reference_t<decltype(boost::pfr::get<obj_i>(T{}))>>
		static void p_inner_serialize(const T& obj, std::array<uint8_t, ser_size>& ret) {
			constexpr size_t ser_inc { sizeof(std::remove_reference_t<decltype(boost::pfr::get<obj_i>(obj))>) };

			size_t tmp_ser_i { ser_i };

			for(const auto e: boost::pfr::get<obj_i>(obj)) {
				static_assert(std::is_scalar_v<decltype(e)>, "Iterable must be of scalar value_type");

				std::generate(
					ret.begin() + tmp_ser_i,
					ret.begin() + tmp_ser_i + sizeof(e),
					[
						index = static_cast<size_t>(0),
						obj_mem = static_cast<const int64_t*>(
							static_cast<const void*>(
								&e
							)
						)
					]() mutable {
						return static_cast<uint8_t>(
							(*obj_mem >> (index++ * 8))
							& 0xFF
						);
					}
				);

				tmp_ser_i += sizeof(e);
			}

			if constexpr(obj_i + 1 < obj_size) {
				p_inner_serialize<T, obj_size, obj_i + 1, ser_size, ser_i + ser_inc>(obj, ret);
			}
		}

		template<typename T>
		static std::array<uint8_t, get_serialized_size<T>()> p_serialize(const T& obj)
		requires (!std::is_empty_v<T>) {
			static_assert(decltype(boost::pfr::detail::tie_as_tuple(obj))::size_v != 0, "const T& obj must not be empty");
			std::array<uint8_t, get_serialized_size<T>()> ret { static_cast<uint8_t>(get_index<T, Args...>()) };
			p_inner_serialize<T, decltype(boost::pfr::detail::tie_as_tuple(obj))::size_v, 0, get_serialized_size<T>(), 1>(obj, ret);
			return ret;
		}

		template<typename T>
		static std::array<uint8_t, get_serialized_size<T>()> p_serialize(const T& obj)
		requires std::is_empty_v<T> {
            (void) obj;
			std::array<uint8_t, get_serialized_size<T>()> ret { static_cast<uint8_t>(get_index<T, Args...>()) };
			return ret;
		}
	public:
		constexpr Serializer() {
			static_assert((sizeof...(Args) < std::numeric_limits<uint8_t>::max()), "Too many arguments");
		}

		template<typename T>
		static auto run(const T& obj) {
			static_assert((std::is_same_v<T, Args> || ...), "Serializer::run: typename T must be member of parameter pack typename ... Args");
			return p_serialize<T>(obj);
		}
	};
}
