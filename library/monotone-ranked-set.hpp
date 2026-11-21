#pragma once
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>

// Initially full set of positions [0, Capacity). Erasures are idempotent.
// Sparse traversal skips empty words and visits surviving ranks in order.
template<std::size_t Capacity>
struct MonotoneRankedSet {
    static_assert(1<=Capacity && Capacity<=4096);
    static constexpr std::size_t Bands=(Capacity+63)/64;
    std::array<std::uint64_t,Bands> active;
    std::uint64_t nonempty;
    MonotoneRankedSet():nonempty(~0ULL>>(64-Bands)) {
        active.fill(~0ULL);
        active.back()>>=64-(Capacity-64*(Bands-1));
    }
    void erase(std::size_t index) {
        assert(index<Capacity);
        auto band=index/64;
        if(!(active[band]&=~(1ULL<<(index%64)))) nonempty&=~(1ULL<<band);
    }
    // Visitor returns true to stop traversal. Do not mutate the set in visitor.
    template<class Visitor>
    bool visit_until(Visitor&& visit) const {
        auto bands=nonempty;
        while(bands) {
            auto b=std::countr_zero(bands);
            bands&=bands-1;
            auto mask=active[b];
            while(mask) {
                auto position=64*b+std::countr_zero(mask);
                mask&=mask-1;
                if(visit(position)) return true;
            }
        }
        return false;
    }
};
