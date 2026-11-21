#include <algorithm>
#include <cassert>
#include <climits>
#include <random>
#include <vector>
#include "../library/hungarian.hpp"

int main() {
    std::mt19937 rng(8102026);
    assert(hungarian_rectangular_minimum_assignment(std::vector<std::vector<int>>{}).empty());
    for (int test=0;test<5000;++test) {
        const int n=1+rng()%5,m=n+rng()%(8-n);
        std::vector matrix(n, std::vector<int>(m));
        for (auto& row:matrix) for (int& c:row) c=int(rng()%201)-100;
        int want=INT_MAX;
        auto dfs=[&](auto&& self,int r,int used,int score)->void {
            if (r==n) { want=std::min(want,score);return; }
            for (int c=0;c<m;++c) if (!(used>>c&1)) self(self,r+1,used|(1<<c),score+matrix[r][c]);
        };
        dfs(dfs,0,0,0);
        const auto assignment=hungarian_rectangular_minimum_assignment(matrix);
        assert(assignment.size()==static_cast<std::size_t>(n));
        int got=0,used=0;
        for (int r=0;r<n;++r) {
            int c=assignment[r];
            assert(0<=c&&c<m&&!(used>>c&1));
            got+=matrix[r][c];used|=1<<c;
        }
        assert(got==want);
        if (n==m) assert(assignment==hungarian_minimum_assignment(matrix));
    }
}
