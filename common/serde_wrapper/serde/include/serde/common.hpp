#pragma once

#include <variant>
#include <cassert>
#include <cstdint>

#include <boost/pfr.hpp>

namespace Serde {
    template<typename T, const size_t index, typename HeadArg, typename ... TailArgs>
    consteval size_t inner_get_index() {
        if constexpr(std::is_same_v<T, HeadArg>) {
            return index;
        } else {
            return inner_get_index<T, index + 1, TailArgs...>();
        }
    }

    template<typename T, typename ... Args>
    consteval size_t get_index() {
        return inner_get_index<T, 0, Args...>();
    }

    template<typename T, const size_t obj_size, const size_t obj_i>
    consteval void inner_get_serialized_size(const T& obj, size_t& ret) {
        ret += sizeof(std::remove_reference_t<decltype(boost::pfr::get<obj_i>(obj))>);
        if constexpr(obj_i + 1 < obj_size) {
            inner_get_serialized_size<T, obj_size, obj_i + 1>(obj, ret);
        }
    }

    template<typename T>
    consteval size_t get_serialized_size()
    requires std::is_empty_v<T> {
        return 1;
    }

    template<typename T>
    consteval size_t get_serialized_size()
	requires (!std::is_empty_v<T>) {
        T obj{};
        static_assert(decltype(boost::pfr::detail::tie_as_tuple(obj))::size_v != 0, "const T& obj must not be empty");
        size_t ret { 0 };
        inner_get_serialized_size<T, decltype(boost::pfr::detail::tie_as_tuple(obj))::size_v, 0>(obj, ret);
        return ret + 1;
    }

    template<typename T, typename = void>
    struct is_iterable : std::false_type {};

    template<typename T>
    struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T&>())),
                                    decltype(std::end(std::declval<T&>()))
                                    >
                    > : std::true_type {};

    template<typename T>
    constexpr bool is_iterable_v = is_iterable<T>::value;

    template<typename ... Args>
    struct pack_holder {
        template <template < typename ... > typename apply_to_T>
        using apply_to = apply_to_T<Args...>;
    };
}