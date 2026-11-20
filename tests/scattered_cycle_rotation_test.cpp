#include "../library/scattered-cycle-rotation.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <vector>

int main() {
    std::mt19937 gen(65833421);
    constexpr int n=80;
    std::vector<int> board(n),index(n),cells(n),seen(n);
    std::iota(board.begin(),board.end(),0);
    std::iota(index.begin(),index.end(),0);
    std::iota(cells.begin(),cells.end(),0);
    for(int it=0;it<12000;++it) {
        std::shuffle(cells.begin(),cells.end(),gen);
        int k=2+gen()%(n-1),d=gen()%2 ? 1 : -1;
        auto old=board,expected=board;
        for(int i=0;i<k;++i) expected[cells[(i+d+k)%k]]=old[cells[i]];
        std::fill(seen.begin(),seen.end(),0);
        rotate_scattered_cycle(board,std::span<const int>(cells.data(),k),d,[&](int at,int value) {
            ++seen[at];index[value]=at;
        });
        assert(board==expected);
        for(int i=0;i<n;++i) assert(index[board[i]]==i);
        for(int i=0;i<n;++i) assert(seen[cells[i]]==(i<k));
    }
}
