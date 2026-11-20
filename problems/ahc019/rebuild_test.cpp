#define AHC019_REBUILD_BLOCKS
#define main ahc019_solver_main
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#include "main.cpp"
#pragma GCC diagnostic pop
#undef main
#include <cassert>
#include <numeric>

int main() {
    Puzzle p{};
    p.d=5; p.n=125;
    p.coord.resize(p.n);
    p.nei.resize(p.n);
    for(int i=0;i<p.n;++i) {
        p.coord[i]={i/25,i/5%5,i%5};
        p.nei[i].fill(-1);  // ray-bias path is still exercised
    }
    for(int s=0;s<2;++s) {
        p.voxels[s].resize(p.n);
        std::iota(p.voxels[s].begin(),p.voxels[s].end(),0);
        p.front[s].fill(31);p.right[s].fill(31);
    }
    Answer original(p.n);
    original.used=6;
    for(int s=0;s<2;++s) for(int block=1;block<=4;++block) {
        for(int x=0;x<=block;++x) original.b[s][p.id(x,block,2+s)]=block;
    }
    original.b[0][0]=5; // unmatched singleton
    original.b[0][1]=6; original.b[1][1]=6;
    std::mt19937 rng(38704);
    for(int k=0;k<50;++k) {
        Answer kept(p.n);
        Fill need(p);
        retain_blocks(p,original,kept,need,rng,k);
        assert(kept.used==3-k%3);
        int old_by_new[5]{};
        int v[2][5]{};
        for(int s=0;s<2;++s) {
            unsigned front[5]{},right[5]{};
            for(int i=0;i<p.n;++i) if(int a=kept.b[s][i]) {
                const int old=original.b[s][i];
                assert(old>0 && old<=4 && a<=kept.used);
                assert(!old_by_new[a] || old_by_new[a]==old);
                old_by_new[a]=old;
                ++v[s][a];
                const auto [x,y,z]=p.coord[i];
                front[z]|=1u<<x;right[z]|=1u<<y;
            }
            for(int z=0;z<p.d;++z) {
                assert(need.front[s][z]==(31^front[z]));
                assert(need.right[s][z]==(31^right[z]));
            }
        }
        long double score=0;
        for(int a=1;a<=kept.used;++a) {
            assert(v[0][a]==v[1][a]&&v[0][a]==old_by_new[a]+1);
            score+=1.0L/v[0][a];
        }
        assert(std::abs(score-kept.cost)<1e-12L);
    }
}
