#include <array>

#include "serde/common.hpp"
#include "serde/serializer.hpp"
#include "serde/deserializer.hpp"

#include "serde/tests.hpp"

namespace Serde {
    namespace Tests {
        template<typename T, typename ... Args>
        constexpr bool test_variant(const std::variant<Args...>& variant) {
            bool ret { false };
            std::visit([&ret](auto&& e) {
                if constexpr(std::is_same_v<T, std::decay_t<decltype(e)>>) {
                    ret = true;
                }
            }, variant);
            return ret;
        }

        template<typename T_Serializer, typename T_Deserializer, typename T>
        bool run(const T& obj) {
            const auto ser { T_Serializer::run(obj) };
            const auto de { T_Deserializer::template run<T>(ser) };

            if(boost::pfr::eq(obj, de) == false) {
                return false;
            }
            
            const auto decoded { T_Deserializer::decode(ser) };
            if(decoded.has_value() == false) {
                return false;
            }

            if(test_variant<T>(decoded.value()) == false) {
                return false;
            }

            if(
                [&obj, &decoded]() {
                    bool ret { false };
                    std::visit([&ret, &obj](auto&& e) {
                        if constexpr(std::is_same_v<std::decay_t<decltype(e)>, T>) {
                            if(boost::pfr::eq(obj, e)) {
                                ret = true;
                            }
                        }
                    }, decoded.value());
                    return ret;
                }()
            == false) {
                return false;
            }

            const auto decoded_by_it { T_Deserializer::decode(ser.cbegin(), ser.cend()) };

            if(decoded_by_it.has_value() == false) {
                return false;
            }

            if(test_variant<T>(decoded_by_it.value()) == false) {
                return false;
            }

            if(
                [&obj, &decoded_by_it]() {
                    bool ret { false };
                    std::visit([&ret, &obj](auto&& e) {
                        if constexpr(std::is_same_v<std::decay_t<decltype(e)>, T>) {
                            if(boost::pfr::eq(obj, e)) {
                                ret = true;
                            }
                        }
                    }, decoded_by_it.value());
                    return ret;
                }()
            == false) {
                return false;
            }

            return true;
        }
    }

    namespace Tests {
        struct Object {
            uint64_t a;
            uint32_t b;
            uint16_t c;
            uint8_t  d;
            int8_t   e;
            int16_t  f;
            int32_t  g;
            int64_t  h;
            float    i;
            double   j;
        };

        const Object object{
            .a = 1234567890,
            .b = 4294967295,
            .c = 65535,
            .d = 132,
            .e = 64,
            .f = -128,
            .g = -32768,
            .h = -9272036854775808,
            .i = 13.12f,
            .j = 66.643,
        };

        enum class Enum {
            First,
            Second,
        };

        struct Super {
            uint64_t a;
            Enum     b;
        };

        const Super super {
            .a = 444,
            .b = Enum::Second
        };
    }

    template<typename ... Args>
    struct Test {

    };

    namespace Tests {
        int object_super() {
            std::array<uint8_t, 12> array;
            const auto array_begin { array.begin() };
            using ObjectSuperPack = pack_holder<Object, Super>;
            using ObjectSuperSerializer = ObjectSuperPack::apply_to<Serde::Serializer>;
            using ObjectSuperDeserializer = ObjectSuperPack::apply_to<Serde::Deserializer>;

            if(run<ObjectSuperSerializer, ObjectSuperDeserializer>(object) == false) {
                return -1;
            }

            if(run<ObjectSuperSerializer, ObjectSuperDeserializer>(super) == false) {
                return -2;
            }

            return 0;
        }
    }

    namespace Tests {
        struct Iterable {
            template<uint8_t ... Indexes> 
            static auto make_array(std::integer_sequence<uint8_t, Indexes...>) -> std::array<uint8_t, sizeof...(Indexes)> {
                return { Indexes... };
            }

            std::array<uint8_t, 12> inner;
        };

        const Iterable iterable {
            .inner = { Iterable::make_array(std::make_integer_sequence<uint8_t, sizeof(Iterable::inner)>()) }
        };
    }

    namespace Tests {
        int object_super_iterable() {
            using ObjectSuperIterablePack = pack_holder<Object, Super, Iterable>;
            using ObjectSuperIterableSerializer = ObjectSuperIterablePack::apply_to<Serde::Serializer>;
            using ObjectSuperIterableDeserializer = ObjectSuperIterablePack::apply_to<Serde::Deserializer>;

            if(run<ObjectSuperIterableSerializer, ObjectSuperIterableDeserializer>(object) == false) {
                return -1;
            }

            if(run<ObjectSuperIterableSerializer, ObjectSuperIterableDeserializer>(super) == false) {
                return -2;
            }

            if(run<ObjectSuperIterableSerializer, ObjectSuperIterableDeserializer>(iterable) == false) {
                return -3;
            }

            return 0;
        }
    }
    
    namespace Tests {
        struct Empty {};
        const Empty empty {};
    }

    namespace Tests {
        int object_super_iterable_empty() {
            using ObjectSuperIterableEmptyPack = pack_holder<Object, Super, Iterable, Empty>;
            using ObjectSuperIterableEmptySerializer = ObjectSuperIterableEmptyPack::apply_to<Serde::Serializer>;
            using ObjectSuperIterableEmptyDeserializer = ObjectSuperIterableEmptyPack::apply_to<Serde::Deserializer>;

            if(run<ObjectSuperIterableEmptySerializer, ObjectSuperIterableEmptyDeserializer>(object) == false) {
                return -1;
            }

            if(run<ObjectSuperIterableEmptySerializer, ObjectSuperIterableEmptyDeserializer>(super) == false) {
                return -2;
            }

            if(run<ObjectSuperIterableEmptySerializer, ObjectSuperIterableEmptyDeserializer>(iterable) == false) {
                return -3;
            }

            if(run<ObjectSuperIterableEmptySerializer, ObjectSuperIterableEmptyDeserializer>(empty) == false) {
                return -4;
            }

            return 0;
        }
    }
}