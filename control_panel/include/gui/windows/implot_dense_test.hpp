#pragma once

#include <array>

class ImPlotDenseTest {
private:
    static constexpr size_t xs_ys_size { 2 << 14 };
    static const std::array<float, xs_ys_size> xs;
    static const std::array<float, xs_ys_size> ys;
public:
    static void draw(bool& enable);
};