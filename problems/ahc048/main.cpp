#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#if !defined(AHC048_2GRAM_POOL) && !defined(AHC048_RECIPES_ONLY) && !defined(AHC048_DILUTION_ONLY)
#define AHC048_RESERVOIR
#endif

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
    int capacity=2;
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
#if defined(AHC048_RESERVOIR) || defined(AHC048_RESERVOIR_ONLY)
    Result run_reservoir() const {
        constexpr int capacity=3;
        const int wells=n*(n/capacity);
        Result result;
        result.capacity=capacity;
        std::vector<Color> stored(wells);
        std::vector<int> volume(wells);
        auto add=[](const Color& color,int mass,const Color& fresh) {
            Color next{};
            for(int i=0;i<3;++i) next[i]=(mass*color[i]+fresh[i])/(mass+1);
            return next;
        };
        struct Recipe {Color color;std::array<int,3> tube{};int grams=0;};
        std::vector<Recipe> recipes;
        for(int a=0;a<k;++a) {
            recipes.push_back({tube[a],{a,-1,-1},1});
            for(int b=a+1;b<k;++b) {
                const auto pair=add(tube[a],1,tube[b]);
                recipes.push_back({pair,{a,b,-1},2});
                for(int c=b;c<k;++c) recipes.push_back({add(pair,2,tube[c]),{a,b,c},3});
                if(b==a+1) continue;
                recipes.push_back({add(tube[a],2,tube[b]),{a,a,b},3});
            }
            if(a+1<k) recipes.push_back({add(tube[a],2,tube[a+1]),{a,a,a+1},3});
        }
        int total=0,poured=0,nonempty=0;
        double total_error=0;
        for(int i=0;i<h;++i) {
            const int left=h-i-1,min_action=t-int(result.ops.size())-2*left;
            const double store_value=d*.65*std::min(1.,double(left)/(6*(total+1)));
            double value=std::numeric_limits<double>::infinity();
            Recipe best;
            int empty=-1,where=-1;
            auto consider=[&](const Recipe& x,int j) {
                const double estimate=1e4*error(x.color,target[i])+
                    d*x.grams-store_value*(x.grams-1);
                if(estimate<value) value=estimate,best=x,where=j;
            };
            for(int j=0;j<wells;++j) {
                const int v=volume[j];
                if(v==0) {
                    if(empty<0) empty=j;
                    continue;
                }
                consider({stored[j],{-1,-1,-1},0},j);
                if(v==capacity || min_action<2) continue;
                for(int a=0;a<k;++a) {
                    const auto next=add(stored[j],v,tube[a]);
                    consider({next,{a,-1,-1},1},j);
                    if(v+2<=capacity && min_action>=3)
                        for(int b=a;b<k;++b) consider({add(next,v+1,tube[b]),{a,b,-1},2},j);
                }
            }
            assert(empty>=0);
            for(const auto& x:recipes)
                if(x.grams+1<=min_action && (x.grams==1 || wells-nonempty>=2))
                    consider(x,empty);
            assert(where>=0);
            for(int k=0;k<best.grams;++k) result.ops.push_back({1,where,best.tube[k]});
            result.ops.push_back({2,where,-1});
            nonempty-=volume[where]>0;
            volume[where]+=best.grams-1;
            nonempty+=volume[where]>0;
            stored[where]=best.color;
            total+=best.grams-1;
            poured+=best.grams;
            total_error+=error(best.color,target[i]);
        }
        assert(total==poured-h && int(result.ops.size())<=t);
        result.score=1+1LL*d*(poured-h)+std::llround(1e4*total_error);
        return result;
    }
#endif

};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Solver s;
    if(!(std::cin>>s.n>>s.k>>s.h>>s.t>>s.d))return 1;
    s.tube.resize(s.k);s.target.resize(s.h);
    for(auto& c:s.tube) for(auto& x:c) std::cin>>x;
    for(auto& c:s.target) for(auto& x:c) std::cin>>x;
#if defined(AHC048_RESERVOIR_ONLY)
    Result best=s.run_reservoir();
#elif defined(AHC048_DILUTION_ONLY)
    Result best=s.run(true);
#elif defined(AHC048_RECIPES_ONLY)
    Result best=s.run(false);
#else
    Result best=s.run(false);
    Result candidate=s.run(true);
    if(candidate.score<best.score) best=std::move(candidate);
#ifdef AHC048_RESERVOIR
    candidate=s.run_reservoir();
    if(candidate.score<best.score) best=std::move(candidate);
#endif
#endif
    const int bins=s.n/best.capacity;
    for(int i=0;i<s.n;++i) for(int j=0;j<s.n-1;++j)
        std::cout<<((j+1)%best.capacity==0 || j+1>=best.capacity*bins)<<(j+2==s.n?'\n':' ');
    for(int i=0;i<s.n-1;++i) for(int j=0;j<s.n;++j)
        std::cout<<1<<(j+1==s.n?'\n':' ');
    for(auto [op,well,tube]:best.ops) {
        const int r=well/bins,c=best.capacity*(well%bins);
        std::cout<<op<<' '<<r<<' '<<c;
        if(op==1) std::cout<<' '<<tube;
        std::cout<<'\n';
    }
    std::cerr<<"Model = "<<best.score<<'\n';
}
