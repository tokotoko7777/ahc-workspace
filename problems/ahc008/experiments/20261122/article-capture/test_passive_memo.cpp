#define main article_program_main_for_speed_test
#include "solve-passive-memo.cpp"
#undef main
int main(){
    Game g;
    std::mt19937 rng(1858620802);
    for(int p=0;p<V;p++)nb[p]=lastcap::neighbors(p);
    auto equal = [&](int p,int type,int h){
        Game::Forecast before{},scratch;
        g.pets={{p,type}};
        g.forecastPet(0,h,before);
        const auto& after=g.cachedPassive(p,type,h,scratch);
        for(int j=0;j<=h;j++){
            assert(std::memcmp(before[j].data(),after[j].data(),V*sizeof(float))==0);
        }
    };
    for(int epoch=0;epoch<120;epoch++){
        g.wall.fill(0);
        for(int j=0;j<50;j++)g.wall[rng()%V]=1;
        g.observedMotionGraph();g.preparePassiveMemo();
        for(int j=0;j<50;j++){
            int p=rng()%V,type=1+rng()%3;
            if(g.wall[p])continue;
            equal(p,type,1+rng()%3);
            equal(p,type,1+rng()%10);
            equal(p,type,1+rng()%8);
        }
    }
    // 300+ independent starts force the no-cache path after the 256-entry cap.
    g.wall.fill(0);g.observedMotionGraph();g.preparePassiveMemo();
    for(int p=0;p<350;p++)equal(p,1,2);
    assert(g.passiveUsed==256);
    std::cout<<"passive cached distributions exactly match full recomputation\n";
}
