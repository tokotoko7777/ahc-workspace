#include "../library/hidden-destination-filter.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <random>

using ahc::HiddenDestinationFilter;
static void check(double x, double y) { assert(std::abs(x-y)<1e-9); }

int main() {
    using Row = std::array<int,4>;
    using Neighbors = std::array<Row,2>;
    auto end2=[](std::size_t x){return x==2;};
    HiddenDestinationFilter<4> line;
    Row from_1{1,0,1,2};
    Neighbors around_1{{Row{0,1,2,3},Row{2,1,0,1}}};
    auto p=line.observe(from_1,around_1,3,1,end2);
    check(p[0],1./3); check(p[1],2./3);
    check(line.redrawn,.5); check(line.mass[3],.5);
    Row from_2{2,1,0,1};
    Neighbors around_2{{Row{1,0,1,2},Row{3,2,1,0}}};
    auto forecast=line.observe(from_2,around_2,3,2,end2);
    check(forecast[0],1./3);check(forecast[1],2./3);check(line.mass[3],.5);

    HiddenDestinationFilter<4> ring;
    Row from_0{0,1,2,1};
    Neighbors around_0{{Row{1,0,1,2},Row{1,2,1,0}}};
    p=ring.observe(from_0,around_0,3,0,[](std::size_t x){return x==1;});
    check(p[0],.5);check(p[1],.5);check(ring.redrawn,2./3);check(ring.mass[2],1./3);
    // A formerly plausible target lies behind a new wall.
    p=ring.observe(Row{1,0,-1,-1},Neighbors{{Row{0,1,-1,-1},Row{}}},1,0,[](auto x){return x==0;});
    check(p[0],1);check(ring.redrawn,1);

    // Compare independently with target/path enumeration on random undirected graphs.
    constexpr int n=8;
    std::mt19937 rng(72431);
    for (int trial=0;trial<3600;++trial) {
        std::array<std::array<int,n>,n> d{};
        for(int i=0;i<n;++i)for(int j=i+1;j<n;++j)d[i][j]=d[j][i]=rng()%5<=1?1:999;
        for(int k=0;k<n;++k)for(int i=0;i<n;++i)for(int j=0;j<n;++j)d[i][j]=std::min(d[i][j],d[i][k]+d[k][j]);
        int start=rng()%n;
        unsigned legal=0;
        for (int i=0;i<n;++i) if (d[start][i]==1) legal|=1u<<i;
        if(!legal) continue;
        auto root=d[start];
        for (int& v:root) if(v>=999)v=-1;
        auto nd=d;
        for(auto& row:nd)for(int& v:row)if(v>=999)v=-1;
        HiddenDestinationFilter<n> f;
        double den=1.;
        for(double& v:f.mass){v=(rng()%101)*.001;den+=v;}
        f.redrawn=1./den;
        for(double& v:f.mass)v/=den;
        const unsigned act=std::countr_zero(legal);
        const int goal=rng()%n;
        auto before=f;
        auto pr=f.observe(root,nd,legal,act,[&](auto v){return v==std::size_t(goal);});
        std::array<double,n> ref{},joint{};
        double restart=before.redrawn;int reachable=0;
        for(int v=0;v<n;++v){
            if(v!=start&&d[start][v]<999)++reachable;
            else restart+=before.mass[v];
        }
        for(int v=0;v<n;++v)if(v!=start&&d[start][v]<999){
            std::array<int,n> dr{};int choices=0;
            for(int k=0;k<n;++k)if(d[start][k]==1&&d[k][v]+1==d[start][v])dr[choices++]=k;
            for(int k=0;k<choices;++k){
                int move=dr[k];double share=(before.mass[v]+restart/reachable)/choices;
                ref[move]+=share;
                if(move==int(act))joint[v]+=share;
            }
        }
        const double event=ref[act];
        assert(event>0);
        for(int k=0;k<n;++k)check(pr[k],ref[k]);
        for(int v=0;v<n;++v)check(f.mass[v],v==goal?0:joint[v]/event);
        check(f.redrawn,joint[goal]/event);
        check(std::accumulate(f.mass.begin(),f.mass.end(),f.redrawn),1.);
    }
}
