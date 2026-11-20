#include "../library/sparse-group-conflicts.hpp"
#include <iostream>

int main() {
    SparseGroupConflicts conflict(3);
    conflict.add(0,1,10); conflict.add(1,2,3);
    std::vector<int> days{0,0,1};
    const auto old=conflict.cost(days), change=conflict.swap_delta(days,1,2);
    std::swap(days[1],days[2]);
    std::cout<<old<<" -> "<<conflict.cost(days)<<'\n';
    return old+change==conflict.cost(days) && change==-10 ? 0 : 1;
}
