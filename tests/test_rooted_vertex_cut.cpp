#include "library/rooted-vertex-cut.hpp"
#include <array>
#include <bitset>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

int main() {
    constexpr int M=7,N=M*M;
    using Mask=std::bitset<N>;
    constexpr auto graph=[]{
        std::array<std::array<int,4>,N> a{};
        for(int v=0;v<N;++v) a[v]={v>=M?v-M:-1,v%M<M-1?v+1:-1,
            v+M<N?v+M:-1,v%M?v-1:-1};
        return a;
    }();
    auto neighbors=[&](int v)->const auto&{return graph[v];};
    ahc::RootedVertexCut<N> solver;
    std::mt19937 rng(991234);
    std::size_t singles=0,pairs=0;
    for(int it=0;it<700;++it) {
        Mask blocked;
        std::array<std::uint64_t,N> label{};
        for(int v=0;v<N;++v){blocked[v]=(rng()%100)<unsigned(it%45);label[v]=1ull<<v;}
        int root=rng()%N;
        blocked.reset(root);
        auto brute=[&](const Mask& w){
            Mask seen; std::array<int,N> q{};std::size_t ends=1;
            seen[root]=1;q[0]=root;
            ahc::RootedCutMass got{};
            for(std::size_t i=0;i<ends;++i) {
                int v=q[i];++got.size;got.labels|=label[v];
                for(int u:graph[v])if(u>=0&&!w[u]&&!seen[u]){
                    seen[u]=1;q[ends++]=u;
                }
            }
            return got;
        };
        auto open=[&](int v){return !blocked[v];};
        assert(solver.build(root,neighbors,open,label));
        assert(solver.whole()==brute(blocked));
        assert(!solver.without(root)&&!solver.without(-1)&&!solver.without(N));
        std::array<ahc::RootedCutMass,N> ref{};
        for(int v=0;v<N;++v)if(v!=root){
            ref[v]=brute(Mask(blocked).set(v));
            assert(solver.without(v)==ref[v]);++singles;
        }
        for(int a=0;a<N;++a)if(a!=root&&!blocked[a]){
            blocked.set(a);
            assert(solver.build(root,neighbors,open,label));
            assert(solver.whole()==ref[a]);
            for(int b=a+1;b<N;++b)if(b!=root){
                assert(solver.without(b)==brute(Mask(blocked).set(b)));++pairs;
            }
            blocked.reset(a);
        }
        assert(!solver.build(-1,neighbors,open,label)&&!solver.without(0));
    }
    std::cout<<singles<<" single and "<<pairs<<" paired deletions match brute BFS\n";
}
