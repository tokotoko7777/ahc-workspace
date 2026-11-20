#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

using Color=std::array<double,3>;
struct Operation {int type,well,tube;};
struct Option {
    Color color{};
    int well=-1,tube=-1,second=-1,used=0;
    bool keeps=false;
    double priority=std::numeric_limits<double>::infinity();
};
struct Result {
    std::vector<Operation> ops;
    long long score=0;
};
static Color mix(const Color& a,const Color& b) {
    return {(a[0]+b[0])*.5,(a[1]+b[1])*.5,(a[2]+b[2])*.5};
}
static double error(const Color& a,const Color& b) {
    double x=a[0]-b[0],y=a[1]-b[1],z=a[2]-b[2];
    return std::sqrt(x*x+y*y+z*z);
}
struct Solver {
    int n,k,h,t,d;
    std::vector<Color> tube,target;
    Result run(bool dilution) const {
        const int wells=n*(n/2);
        std::vector<Color> cache(wells);
        std::vector<unsigned char> occupied(wells);
        struct Recipe {Color color;int a,b;};
        std::vector<Recipe> recipes;
        for(int a=0;a<k;++a) for(int b=a+1;b<k;++b)
            recipes.push_back({mix(tube[a],tube[b]),a,b});
        Result result;
        result.ops.reserve(3*h);
        double cumulative_error=0;
        int total_poured=0,stock=0;
        for(int i=0;i<h;++i) {
            int empty=-1;
            for(int j=0;j<wells;++j) if(!occupied[j]) {empty=j;break;}
            assert(empty>=0);
            const Color& wanted=target[i];
            const int spare=wells-stock;
            const int left=h-i-1;
            const int their_turns=t-int(result.ops.size())-2*left;
            // The value of creating one spare gram decays as the collection
            // fills up or the remaining target list becomes short.
            const double value_stock=d*.65*
                std::min(1.,double(left)/(6*(stock+1)));
            Option best;
            auto consider=[&](Color color,int j,int a,int b,int used,bool keeps) {
                if(used+1>their_turns) return;
                const int stock_diff=int(keeps)-int(occupied[j]);
                const double priority=1e4*error(color,wanted)+d*used-value_stock*stock_diff;
                if(priority<best.priority) best={color,j,a,b,used,keeps,priority};
            };
            for(int a=0;a<k;++a) consider(tube[a],empty,a,-1,1,false);
            if(spare>=2) for(auto [color,a,b]:recipes)
                consider(color,empty,a,b,2,true);
            for(int j=0;j<wells;++j) if(occupied[j]) {
                consider(cache[j],j,-1,-1,0,false);
                if(dilution) for(int a=0;a<k;++a)
                    consider(mix(cache[j],tube[a]),j,a,-1,1,true);
            }
            assert(best.well>=0);
            if(best.tube>=0) result.ops.push_back({1,best.well,best.tube});
            if(best.second>=0) result.ops.push_back({1,best.well,best.second});
            result.ops.push_back({2,best.well,-1});
            stock+=int(best.keeps)-int(occupied[best.well]);
            occupied[best.well]=best.keeps;
            if(best.keeps) cache[best.well]=best.color;
            total_poured+=best.used;
            cumulative_error+=error(best.color,wanted);
        }
        assert(total_poured-h==stock && int(result.ops.size())<=t);
        result.score=1+1LL*d*(total_poured-h)+std::llround(1e4*cumulative_error);
        return result;
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Solver s;
    if(!(std::cin>>s.n>>s.k>>s.h>>s.t>>s.d))return 1;
    s.tube.resize(s.k);s.target.resize(s.h);
    for(auto& c:s.tube) for(auto& x:c) std::cin>>x;
    for(auto& c:s.target) for(auto& x:c) std::cin>>x;
#ifdef AHC048_DILUTION_ONLY
    Result best=s.run(true);
#elif defined(AHC048_RECIPES_ONLY)
    Result best=s.run(false);
#else
    Result best=s.run(false);
    Result candidate=s.run(true);
    if(candidate.score<best.score) best=std::move(candidate);
#endif
    for(int i=0;i<s.n;++i) for(int j=0;j<s.n-1;++j)
        std::cout<<((j&1) || j+1>=2*(s.n/2))<<(j+2==s.n?'\n':' ');
    for(int i=0;i<s.n-1;++i) for(int j=0;j<s.n;++j)
        std::cout<<1<<(j+1==s.n?'\n':' ');
    for(auto [op,well,tube]:best.ops) {
        const int r=well/(s.n/2),c=2*(well%(s.n/2));
        std::cout<<op<<' '<<r<<' '<<c;
        if(op==1) std::cout<<' '<<tube;
        std::cout<<'\n';
    }
    std::cerr<<"Model = "<<best.score<<'\n';
}
