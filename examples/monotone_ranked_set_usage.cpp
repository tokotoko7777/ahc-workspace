#include <iostream>
#include "../library/monotone-ranked-set.hpp"
int main() {
    MonotoneRankedSet<130> ranks;
    ranks.erase(0);
    ranks.erase(1);
    ranks.visit_until([](auto rank) {
        std::cout << rank << '\n'; // first still-active rank
        return true;
    });
}
