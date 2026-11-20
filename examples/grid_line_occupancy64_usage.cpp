#include "../library/grid-line-occupancy64.hpp"
#include <iostream>

int main() {
    GridLineOccupancy64 occupied(3,5);
    occupied.set(1,4,true);
    occupied.set(0,2,true);
    auto [up,down,left,right]=occupied.visible(1,2);
    std::cout<<up<<' '<<down<<' '<<left<<' '<<right<<'\n'; // 2 -1 -1 9
    return up==2 && down==-1 && left==-1 && right==9 ? 0 : 1;
}
