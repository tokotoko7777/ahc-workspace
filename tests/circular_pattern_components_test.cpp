#include "../library/weighted-dsu.hpp"
#include "../library/circular-pattern-components.hpp"
#include <cassert>
#include <random>

int main(){
    std::mt19937 rng(89651);
    for(int t=0;t<700;++t) {
        const int n=int(rng()%19)+1, m=int(rng()%38)+1;
        std::string ring(n,'.');
        for(char& c:ring)c=char('A'+rng()%8);
        std::vector<int> origin(m);
        std::vector<std::string> slices(m);
        for(int i=0;i<m;++i) {
            origin[i]=int(rng()%n);
            const int len=int(rng()%n)+1;
            for(int k=0;k<len;++k)slices[i]+=k>0 && rng()%5==0 ? '.' : ring[(origin[i]+k)%n];
        }
        CircularPatternComponents c(n,slices);
        for(int step=0;step<3*m;++step){
            int a=int(rng()%m), b=int(rng()%m);
            assert(c.join(a,b,origin[b]-origin[a]+(int(rng()%9)-4)*n));
            const int root=c.leader(a);
            assert(root==c.leader(b));
            if(n>1) assert(!c.join(a,b,origin[b]-origin[a]+1));
            for(int i=0;i<m;++i) {
                int r=c.leader(i);
                assert(c.offset(i)==(origin[i]-origin[r]+n)%n);
                const auto& p=c.pattern(i);
                for(int k=0;k<n;++k)
                    assert(p[k]=='.'||p[k]==ring[(origin[r]+k)%n]);
                for(int k=0;k<int(slices[i].size());++k)
                    if(slices[i][k]!='.')assert(slices[i][k]==p[(c.offset(i)+k)%n]);
            }
        }
    }
    CircularPatternComponents c(3,{"AB","AC","BC"});
    assert(c.join(0,2,1));
    assert(!c.join(0,1,0));
    assert(c.leader(0)!=c.leader(1));
    assert(c.pattern(0)=="ABC");
    assert(c.pattern(1)=="AC.");
}
