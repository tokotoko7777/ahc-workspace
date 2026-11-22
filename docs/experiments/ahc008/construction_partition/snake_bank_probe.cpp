#pragma GCC optimize("O3")
#include "route_makespan.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;
using route_makespan::Cost;
static constexpr int S=30, V=900, INF=99999;
int id(int x,int y){return x*S+y;}
int md(int x,int y){return abs(x/S-y/S)+abs(x%S-y%S);}
array<int,4> ns(int p){return {p>=30?p-30:-1,p+30<900?p+30:-1,p%30?p-1:-1,p%30<29?p+1:-1};}
struct Graph {
    array<bool,V> wall{};
    array<int,V> bfs(int start) const {
        array<int,V> d;d.fill(INF);d[start]=0;
        array<int,V> q{};int h=0,t=1;q[0]=start;
        while(h<t){
            int v=q[h++];
            for(int n:ns(v))if(n>=0&&!wall[n]&&d[n]==INF){d[n]=d[v]+1;q[t++]=n;}
        }
        return d;
    }
};

int main(){
 cin.tie(nullptr);ios::sync_with_stdio(false);
 int n,m;if(!(cin>>n))return 1;
 vector<pair<int,int>> pets(n);
 for(auto& [p,t]:pets){int x,y;cin>>x>>y>>t;p=id(x-1,y-1);}
 cin>>m;vector<int> humans(m);
 for(auto& p:humans){int x,y;cin>>x>>y;p=id(x-1,y-1);}
 // The original three-bank pilot's visible starting geometry choice.
 double best=-1e100;int rot=0,ax=0;
 for(int z=0;z<2;++z)for(int a:{2,4,6}){
     double f=0;
     for(auto [p,t]:pets){
       int x=(z?p%S:p/S),y=(z?p/S:p%S),d=99;
       if(x<2||x>26)continue;
       for(int b=0;b<3;++b)for(int k=2;k<=5;++k)d=min(d,abs(y-(a+7*b+k)));
       f+=1./(1.+.8*d);
     }
     for(int b=0;b<4;++b){
       const int c=a+7*b;
       const int target=z?id(c,14):id(14,c);
       int nearest=99;for(int h:humans)nearest=min(nearest,md(h,target));
       f-=.045*nearest;
     }
     if(f>best){best=f;rot=z;ax=a;}
 }
 auto at=[&](int x,int y){return rot?id(y,x):id(x,y);};
 Graph g;
 for(int b=0;b<3;++b)for(int x=2;x<=26;x+=2)for(int s=2;s<=5;++s)g.wall[at(x,ax+7*b+s)]=true;
 vector<array<int,V>> human;
 for(int h:humans)human.push_back(g.bfs(h));
 Cost opt=INF;int recipe=-1,maxjob=0,totalwork=0,walkcost=0;
 // Six paired passes plus one standalone brace per bank. All stances and
 // connecting routes remain open even when every brace has been built.
 for(int mode=0;mode<16;++mode){
    vector<pair<int,array<int,2>>> sweeps;
    auto block=[&](int i,int j){sweeps.push_back({j, i==6?array<int,2>{25,1}:array<int,2>{3+4*i,3}});};
    if(mode&1) {
         for(int i=0;i<7;++i)for(int j=0;j<3;++j)block((mode&4)?6-i:i, (i&1)?2-j:j);
    }else{
         for(int j=0;j<3;++j)for(int i=0;i<7;++i)block((j&1)?6-i:i,(mode&4)?2-j:j);
    }
    if(mode&8)reverse(sweeps.begin(),sweeps.end());
    vector<int> stands;vector<Cost> work,walk;
    for(int z=0;z<(int)sweeps.size();++z){
        auto [j,p]=sweeps[z];
        auto [r,w]=p;
        // Pair above and below except the final brace r=26 is built
        // from r=25 (one operation). All columns in each stroke are open.
        for(int i=0;i<4;++i){
            int col=ax+7*j+2+(((z&1)^(bool(mode&2)))?3-i:i);
            stands.push_back(at(r,col));work.push_back(w==3?2:1);
        }
    }
    for(int i=0;i<(int)stands.size();++i){
      if(g.wall[stands[i]])return 2;
      walk.push_back(i+1==(int)stands.size()?0:g.bfs(stands[i])[stands[i+1]]);
    }
    vector<vector<Cost>> approach(m);
    for(int h=0;h<m;++h)for(int stand:stands)approach[h].push_back(human[h][stand]);
    auto plan=route_makespan::solve(work,walk,approach);
    if(plan.makespan<opt){
        opt=plan.makespan;recipe=mode;
        totalwork=0;walkcost=0;maxjob=0;
        for(auto [h,l,r]:plan.segments){
            maxjob=max(maxjob,r-l);
            walkcost+=approach[h][l];
            for(int k=l;k<r;++k){totalwork+=work[k];if(k+1<r)walkcost+=walk[k];}
        }
    }
 }
 if(totalwork!=156 || opt==INF)return 3;
 cout<<m<<','<<rot<<','<<ax<<','<<opt<<','<<recipe<<','<<walkcost<<','<<maxjob<<'\n';
}
