#include "../library/orthogonal-slide-grid.hpp"
#include <cmath>
#include <random>
int main() {
    std::mt19937 rng(500904);
    constexpr int dr[]{0,0,-1,1},dc[]{-1,1,0,0};
    for(int test=0;test<100;++test) {
        int h=1+rng()%9,w=1+rng()%9,n=h*w;
        std::vector<unsigned char> block(n);
        OrthogonalSlideGrid graph(h,w,block);
        std::vector<double> p(n),next(n),expected(n);
        for(int rep=0;rep<125;++rep) {
            int change=rng()%n;
            block[change]^=1;
            graph.set_blocked(change,block[change]);
            std::fill(expected.begin(),expected.end(),0);
            for(int v=0;v<n;++v) {
                p[v]=block[v]?0.:double(rng()%65)/256;
                assert(graph.blocked(v)==bool(block[v]));
                for(int d=0;d<4;++d) {
                    int r=v/w,c=v%w;
                    if(!block[v]) while(true) {
                        const int nr=r+dr[d],nc=c+dc[d];
                        if(nr<0||nr>=h||nc<0||nc>=w||block[nr*w+nc]) break;
                        r=nr;c=nc;
                    }
                    const int end=r*w+c;
                    assert(graph.destinations(v)[d]==end);
                    expected[end]+=p[v]/4.;
                }
            }
            graph.redistribute(p,next);
            assert(expected==next);
        }
    }
}
