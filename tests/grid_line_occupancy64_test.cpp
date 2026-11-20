#include "../library/grid-line-occupancy64.hpp"
#include <array>
#include <cassert>
#include <random>

int main() {
    std::mt19937 rng(781309);
    constexpr int dr[]={-1,1,0,0}, dc[]={0,0,-1,1};
    for(auto [h,w] : {std::pair{1,1}, {1,64}, {64,1}, {64,64}, {31,17}, {4,42}}) {
        GridLineOccupancy64 index(h,w);
        std::array<std::array<bool,64>,64> grid{};
        for(int update=0;update<4096;++update) {
            int y=rng()%h,x=rng()%w;
            bool value=rng()%2;
            grid[y][x]=value; index.set(y,x,value);
            for(int t=0;t<4;++t) {
                int r=rng()%h,c=rng()%w;
                std::array<int,4> expected{};
                expected.fill(-1);
                for(int a=0;a<4;++a)
                    for(int yy=r+dr[a],xx=c+dc[a];0<=yy&&yy<h&&0<=xx&&xx<w;yy+=dr[a],xx+=dc[a])
                        if(grid[yy][xx]) {expected[a]=yy*w+xx;break;}
                assert(index.visible(r,c)==expected);
            }
        }
    }
}
