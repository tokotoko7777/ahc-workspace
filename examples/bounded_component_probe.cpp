#include "../library/bounded-component-probe.hpp"

#include <array>
#include <bitset>
#include <cassert>

int main() {
    // 0--1   2--3--4. Only 4 is a relevant destination.
    const std::array links{
        std::array{1, -1}, std::array{0, -1}, std::array{3, -1},
        std::array{2, 4}, std::array{3, -1}};
    const auto next = [&](int v) -> const auto& { return links[v]; };
    const auto allowed = [](int) { return true; };
    const auto destination = [](int v) { return v == 4; };
    std::bitset<5> proved;
    using S = ahc::ComponentProbe;
    const auto probe = [&](int start) {
        return ahc::bounded_component_probe<5, 2>(start, next, allowed, destination, proved);
    };
    assert(probe(0) == S::disjoint);
    assert(probe(1) == S::disjoint);  // Cached from the complete first component.
    assert(probe(2) == S::intersects);
    assert(probe(4) == S::intersects);
}
