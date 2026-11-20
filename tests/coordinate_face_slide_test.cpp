#include "../library/coordinate-face-slide.hpp"
#include <array>
#include <cassert>
#include <random>

using Rect=std::array<int,4>;
bool overlap(const Rect& a,const Rect& b) {
    return std::max(a[0],b[0])<std::min(a[2],b[2])
        && std::max(a[1],b[1])<std::min(a[3],b[3]);
}

int main() {
    std::mt19937 rng(123456);
    constexpr int W=18;
    for(int rep=0;rep<800;++rep) {
        std::vector<Rect> r;
        // Mix shared contact edges with gaps and unconstrained rectangles.
        if(rep%2==0) {
            const int x=3+rng()%12,y=3+rng()%12;
            r={{0,0,x,y},{0,y,x,W},{x,0,W,W}};
        }
        for(int k=0;k<36;++k) {
            const int x=rng()%W,y=rng()%W,w=1+rng()%5,h=1+rng()%5;
            Rect a{x,y,std::min(W,x+w),std::min(W,y+h)};
            if(std::none_of(r.begin(),r.end(),[&](const Rect& b){return overlap(a,b);})) r.push_back(a);
        }
        const int n=int(r.size());
        std::vector<std::array<int,2>> point;
        for(const auto& a:r) point.push_back({a[0]+int(rng()%(a[2]-a[0])),a[1]+int(rng()%(a[3]-a[1]))});
        auto get=[&](int i,int d){return r[i][d];};
        auto extra=[&](int i,int d) {
            int p=point[i][d%2];
            return d<2?std::pair{0,p}:std::pair{p+1,W};
        };
        for(int axis=0;axis<2;++axis) {
            auto groups=coordinate_face_slide::build(n,axis,get);
            std::vector<unsigned char> mark;
            int cnt=0;
            for(const auto& g:groups) {
                cnt+=g.size();
                const int old=get(g.front()/4,g.front()%4);
                for(int f:g) assert(old==get(f/4,f%4));
                const auto [lo,hi]=coordinate_face_slide::limits(n,axis,get,extra,g,0,W,mark);
                assert(lo<=old && old<=hi);
                for(int z=0;z<=W;++z) {
                    auto t=r;
                    for(int f:g)t[f/4][f%4]=z;
                    bool pass=true;
                    for(int i=0;i<n;++i) {
                        auto [l,b,u,v]=t[i]; auto [x,y]=point[i];
                        if(!(0<=l && l<=x && x<u && u<=W && 0<=b && b<=y && y<v && v<=W)) pass=false;
                        for(int j=0;j<i;++j) if(overlap(t[i],t[j])) pass=false;
                    }
                    assert(pass==(lo<=z && z<=hi));
                }
            }
            assert(cnt==2*n);
        }
    }
}
