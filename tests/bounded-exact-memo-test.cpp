#include "bounded-exact-memo.hpp"
#include <array>
#include <cassert>
#include <cstdlib>
#include <random>
int main(){
    using Key=std::array<int,3>;
    ahc::BoundedExactMemo<Key,int,16> memo;
    std::array<bool,16> set{};
    std::array<Key,16> state{};
    std::mt19937 rng(374860);
    int misses=0,hits=0,income=4;
    for(int t=0;t<12000;++t){
        if(t%1500==0){memo.clear();set.fill(false);++income;}
        unsigned h=t%65<51 ? rng()%3 : rng()%16;
        Key k={int(rng()%4),int(rng()%3),int(rng()%5)};
        if(t%9==0 && set[h])k=state[h];
        int want=100*k[0]-17*k[1]+k[2]*k[2]+income;
        bool made=false, expected=set[h]&&state[h]==k;
        auto [value,hit]=memo.get_or_compute(k,h,[&] {made=true;return want;});
        assert(hit==expected && hit!=made && *value==want);
        hits+=hit;misses+=made;
        set[h]=true;state[h]=k;
    }
    assert(hits>=1200&&misses>=1200);
}
