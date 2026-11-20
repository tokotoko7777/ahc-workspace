#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "../../library/cube-rotations.hpp"

using cube_rotations::Point;
constexpr int max_d=14;
struct Puzzle {
    int d,n;
    std::array<std::array<unsigned,max_d>,2> front{},right{};
    std::array<std::vector<int>,2> allowed,voxels;
    std::vector<std::array<int,6>> nei;
    std::vector<Point> coord;
    std::array<std::array<int,6>,24> rot_dir{};
    static constexpr std::array<Point,6> steps{{{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}}};

    int id(int x,int y,int z) const {return (x*d+y)*d+z;}
    bool read() {
        if(!(std::cin>>d)) return false;
        n=d*d*d;
        for(int s=0;s<2;++s) for(auto* view:{&front[s],&right[s]})
            for(int z=0;z<d;++z) {
                std::string r;std::cin>>r;
                for(int j=0;j<d;++j) if(r[j]=='1') (*view)[z]|=1u<<j;
            }
        nei.resize(n);coord.resize(n);
        for(auto& side:allowed) side.assign(n,0);
        for(int i=0;i<n;++i) {
            const int x=i/(d*d),y=i/d%d,z=i%d;
            coord[i]={x,y,z};
            for(int s=0;s<2;++s) if((front[s][z]>>x&1)&&(right[s][z]>>y&1)) {
                allowed[s][i]=1;voxels[s].push_back(i);
            }
            for(int a=0;a<6;++a) {
                auto [dx,dy,dz]=steps[a];
                auto valid=[&](int k){return 0<=k&&k<d;};
                nei[i][a]=valid(x+dx)&&valid(y+dy)&&valid(z+dz)?id(x+dx,y+dy,z+dz):-1;
            }
        }
        for(int r=0;r<24;++r) for(int a=0;a<6;++a) {
            Point p=cube_rotations::all[r](steps[a]);
            rot_dir[r][a]=std::find(steps.begin(),steps.end(),p)-steps.begin();
        }
        return true;
    }
};
struct Answer {
    int used=0;
    long double cost=0;
    std::array<std::vector<int>,2> b;
    explicit Answer(int n) {for(auto& row:b) row.assign(n,0);}
};
struct Fill {
    std::array<std::array<unsigned,max_d>,2> front,right;
    explicit Fill(const Puzzle& p):front(p.front),right(p.right){}
    bool complete(int d) const {
        for(int s=0;s<2;++s) for(int z=0;z<d;++z)
            if(front[s][z]||right[s][z]) return false;
        return true;
    }
    void add(int s,const Point& p) {
        const auto [x,y,z]=p;
        front[s][z]&=~(1u<<x);right[s][z]&=~(1u<<y);
    }
};

void finish(const Puzzle& p,const Fill& f,Answer& ans) {
    std::array<std::vector<int>,2> singles;
    for(int s=0;s<2;++s) for(int z=0;z<p.d;++z) {
        unsigned x=f.front[s][z],y=f.right[s][z];
        const int default_x=std::countr_zero(p.front[s][z]),default_y=std::countr_zero(p.right[s][z]);
        while(x||y) {
            const int a=x?std::countr_zero(x):default_x;
            const int b=y?std::countr_zero(y):default_y;
            singles[s].push_back(p.id(a,b,z));
            if(x) x&=x-1;
            if(y) y&=y-1;
        }
    }
    const int cost=std::max(singles[0].size(),singles[1].size());
    for(int i=0;i<cost;++i) {
        ++ans.used;
        for(int s=0;s<2;++s) if(i<int(singles[s].size())) ans.b[s][singles[s][i]]=ans.used;
    }
    ans.cost+=cost;
}

#ifndef AHC019_SINGLETON_ONLY
Answer solve_blocks(const Puzzle& p,const Answer& upper) {
    using Match=std::array<int,2>;
    Answer best=upper;
    std::vector<int> seen(p.n);
    std::vector<Match> component,placement;
    component.reserve(p.n);placement.reserve(p.n);
    int stamp=0;
    std::mt19937 rng(3109759);
    const auto stop=std::chrono::steady_clock::now()+std::chrono::milliseconds(1300);
    for(int restart=0;restart<15&&std::chrono::steady_clock::now()<stop;++restart) {
        Answer now(p.n);Fill need(p);
        for(int cnt=0;cnt<64&&!need.complete(p.d)&&std::chrono::steady_clock::now()<stop;++cnt) {
            std::array<std::vector<int>,2> free,new_ray;
            int deficit[2]{};
            for(int s=0;s<2;++s) {
                for(int i:p.voxels[s]) if(!now.b[s][i]) {
                    free[s].push_back(i);
                    const auto [x,y,z]=p.coord[i];
                    if((need.front[s][z]>>x&1)||(need.right[s][z]>>y&1)) new_ray[s].push_back(i);
                }
                for(int z=0;z<p.d;++z)
                    deficit[s]+=std::max(std::popcount(need.front[s][z]),std::popcount(need.right[s][z]));
            }
            if(free[0].empty()||free[1].empty()) break;
            placement.clear();
            double best_value=-1e100;
            const int trials=cnt<2 ? 550 : 270;
            for(int trial=0;trial<trials;++trial) {
                if((trial&31)==0&&std::chrono::steady_clock::now()>=stop) break;
                Match anchor{};
                for(int s=0;s<2;++s) {
                    const auto& list=(trial%5==0||new_ray[s].empty())?free[s]:new_ray[s];
                    anchor[s]=list[rng()%list.size()];
                }
                int rot=rng()%24;
                ++stamp;
                seen[anchor[0]]=stamp;
                Fill after=need;
                component.clear();component.push_back(anchor);
                for(std::size_t h=0;h<component.size();++h) {
                    auto [a,b]=component[h];
                    after.add(0,p.coord[a]);after.add(1,p.coord[b]);
                    for(int dir=0;dir<6;++dir) {
                        int x=p.nei[a][dir],y=p.nei[b][p.rot_dir[rot][dir]];
                        if(x<0||y<0||seen[x]==stamp) continue;
                        seen[x]=stamp;
                        if(!p.allowed[0][x]||!p.allowed[1][y]||now.b[0][x]||now.b[1][y]) continue;
                        component.push_back({x,y});
                    }
                }
                if(component.size()<2) continue;
                int rest[2]{};
                for(int s=0;s<2;++s) for(int z=0;z<p.d;++z)
                    rest[s]+=std::max(std::popcount(after.front[s][z]),std::popcount(after.right[s][z]));
                int improvement=deficit[0]+deficit[1]-rest[0]-rest[1];
                if(improvement==0) continue;
                int max_reduction=std::max(deficit[0],deficit[1])-std::max(rest[0],rest[1]);
                double value=10.*max_reduction+(2.+restart%4)*improvement
                            +(0.02+0.05*(restart%3))*double(component.size())
                            +std::generate_canonical<double,32>(rng)*0.25;
                if(value>best_value) {best_value=value;placement=component;}
            }
            if(placement.empty()) break;
            ++now.used;
            now.cost+=1.0L/placement.size();
            for(auto [x,y]:placement) {
                now.b[0][x]=now.b[1][y]=now.used;
                need.add(0,p.coord[x]);need.add(1,p.coord[y]);
            }
            if(now.cost>=best.cost) break;
        }
        finish(p,need,now);
        if(now.cost<best.cost) best=std::move(now);
    }
    return best;
}
#endif

int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Puzzle p;
    if(!p.read()) return 1;
    Answer ans(p.n);
    finish(p,Fill(p),ans);
#ifndef AHC019_SINGLETON_ONLY
    ans=solve_blocks(p,ans);
#endif
    std::cout<<ans.used<<'\n';
    for(const auto& row:ans.b) for(int i=0;i<p.n;++i)
        std::cout<<row[i]<<(i+1==p.n?'\n':' ');
}
