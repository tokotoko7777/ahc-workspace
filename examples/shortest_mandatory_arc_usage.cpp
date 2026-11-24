#include "shortest-mandatory-arc.hpp"
#include <array>
#include <cassert>
#include <iostream>

int main() {
    using Index = ahc::shortest_mandatory_arc;
    // Two shortest branches merge before a shared final arc.
    const std::array<Index::arc, 5> edges{Index::arc{0,1}, {0,2}, {1,3}, {2,3}, {3,4}};
    Index table(5, edges, 4);
    assert(table.distance(0) == 3);
    assert(table.mandatory(0, 0).value() == false);
    assert(table.mandatory(0, 4).value() == true);
    assert(table.mandatory(4, 4).value() == false);
    std::cout << "shared final arc is mandatory\n";
}
