#pragma once

#include <array>
#include <bit>
#include <cassert>
#include <cstdint>

// Four-neighbor connected components in an H x W grid, one uint32_t mask
// per row. Set bits are traversable. Low bit is column 0.
template<int H, int W>
struct BitGridComponents {
    static_assert(1 <= H && H <= 32 && 1 <= W && W <= 32);
    using Rows = std::array<std::uint32_t, H>;

    // Consume the component containing the row-major first available cell.
    // Pass distinct input/output arrays and keep bits above column W-1 clear.
    // Returns false and zeroes result when there are no remaining cells.
    static bool pop_first(Rows& remaining, Rows& result, int& count) {
        assert(&remaining != &result);
        result.fill(0);
        count = 0;
        int first = 0;
        while (first < H && !remaining[first]) ++first;
        if (first == H) return false;
        const unsigned col = std::countr_zero(remaining[first]);
        result[first] = expand(std::uint32_t(1) << col, remaining[first]);
        std::uint32_t pending = std::uint32_t(1) << first;
        while (pending) {
            const int r = std::countr_zero(pending);
            pending &= pending - 1;
            auto spread = [&](int i) {
                const auto add = result[r] & remaining[i] & ~result[i];
                if (add) {
                    result[i] |= expand(add, remaining[i]);
                    pending |= std::uint32_t(1) << i;
                }
            };
            if constexpr (H > 1) {
                if (r > 0) spread(r - 1);
                if (r + 1 < H) spread(r + 1);
            }
        }
        for (int r = 0; r < H; ++r) {
            remaining[r] &= ~result[r];
            count += std::popcount(result[r]);
        }
        return true;
    }

private:
    static std::uint32_t expand(std::uint32_t seeds, std::uint32_t allowed) {
        auto l = seeds, r = seeds, lm = allowed, rm = allowed;
        for (int step = 1; step < W; step *= 2) {
            l |= lm & (l << step);
            r |= rm & (r >> step);
            lm &= lm << step;
            rm &= rm >> step;
        }
        return l | r;
    }
};
