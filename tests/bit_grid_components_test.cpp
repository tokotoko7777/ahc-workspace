#include <array>
#include <cassert>
#include <random>
#include <vector>
#include "../library/bit-grid-components.hpp"

template<int H,int W>
void validate(std::mt19937& rng) {
    using Grid = BitGridComponents<H,W>;
    for(int test=0;test<1200;test++){
        typename Grid::Rows available{},comp{};
        std::array<bool,H*W> missing{};
        for(int p=0;p<H*W;++p)if(int(rng()%101)>test%101) {
            available[p/W]|=std::uint32_t(1)<<(p%W);
            missing[p]=true;
        }
        for(int p=0;p<H*W;++p)if(missing[p]){
            std::vector<int> q{p};missing[p]=false;
            typename Grid::Rows expect{};
            for(std::size_t i=0;i<q.size();++i){
                int v=q[i],r=v/W,c=v%W;
                expect[r]|=std::uint32_t(1)<<c;
                for(int u:{r?v-W:-1,r<H-1?v+W:-1,c?v-1:-1,c<W-1?v+1:-1})
                    if(u>=0&&missing[u])missing[u]=false,q.push_back(u);
            }
            int n=0;
            assert(Grid::pop_first(available,comp,n));
            assert(n==(int)q.size()&&comp==expect);
        }
        int n=0;
        assert(!Grid::pop_first(available,comp,n)&&n==0);
    }
}

int main(){
    std::mt19937 rng(8302026);
    validate<1,1>(rng);
    validate<1,32>(rng);
    validate<32,1>(rng);
    validate<7,19>(rng);
    validate<30,30>(rng);
    validate<32,32>(rng);
}
