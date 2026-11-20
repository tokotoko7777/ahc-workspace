#define main ahc071_solver_main
#include "../problems/ahc071/main.cpp"
#undef main
#include <cassert>
#include <random>

struct Row {
    Mask occupied,centers;
    int cost;
};

int main() {
    std::mt19937 rng(71196351);
    for(int rep=0;rep<1200;++rep) {
        Problem p;
        p.W=1+rng()%9;
        p.H=2;
        for(int l:p.widths) if(l) p.cost[l]=1+rng()%20;
        const Mask need_a=rng()%(1<<p.W),need_b=rng()%(1<<p.W);
        std::vector<Row> rows;
        auto enumerate=[&](auto&& self,int x,Mask old,Mask centers,int cost)->void {
            if(x==p.W) {rows.push_back({old,centers,cost});return;}
            self(self,x+1,old,centers,cost);
            for(int l:p.widths) if(l && x+l<=p.W)
                self(self,x+l,old|(((Mask(1)<<l)-1)<<x),centers|(Mask(1)<<(x+(l-1)/2)),cost+p.cost[l]);
        };
        enumerate(enumerate,0,0,0,0);
        Mask foundation=(rep%2) ? ~Mask(0) : ~(Mask(1)<<(rng()%p.W));
        int expected=1<<30;
        for(const Row& a:rows) if((a.occupied&need_a)==need_a)
            for(const Row& b:rows) if((b.centers&foundation)==b.centers &&
                                      ((need_b|a.centers)&b.occupied)==(need_b|a.centers))
                expected=std::min(expected,a.cost+b.cost);
        if(expected==(1<<30)) continue;
        Plan actual;
        Problem::Kernel kernel(p.cost);
        const Mask below=p.two(actual,1,need_a,need_b,kernel,foundation);
        Mask covers[2]{},support[2]{};
        int cost=0;
        for(auto [x,y,l]:actual.bricks) {
            assert(y>=0 && y<=1 && x>=0 && x+l<=p.W);
            const Mask occupied=((Mask(1)<<l)-1)<<x;
            assert((covers[y]&occupied)==0);
            covers[y]|=occupied;
            support[y]|=Mask(1)<<(x+(l-1)/2);
            cost+=p.cost[l];
        }
        assert((covers[1]&need_a)==need_a);
        assert((covers[0]&(need_b|support[1]))==(need_b|support[1]));
        assert((below&foundation)==below && below==support[0] && cost==expected && actual.cost==expected);
    }
}
