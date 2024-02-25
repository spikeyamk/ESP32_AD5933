#include <variant>

template<typename T_Decay>
bool variant_tester(const auto& variant) {
    bool result = false;
    std::visit([&result](auto&& active_state) {
        if constexpr(std::is_same_v<std::decay_t<decltype(active_state)>, T_Decay>) {
            result = true;
        }
    }, variant);
    return result;
}