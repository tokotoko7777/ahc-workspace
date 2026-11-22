#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
using Pos = pair<int, int>;
constexpr int D=30;
constexpr array<Pos,4> dirs={{{-1,0},{1,0},{0,-1},{0,1}}};
Pos add(Pos p, Pos d){return {p.first+d.first,p.second+d.second};}
bool board(Pos p){return 0<=p.first&&p.first<D&&0<=p.second&&p.second<D;}
int md(Pos p,Pos q){return abs(p.first-q.first)+abs(p.second-q.second);}

int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    int n,m;
    if(!(cin>>n)) return 0;
    vector<Pos> pets(n);
    for(auto& [r,c]:pets){int type;cin>>r>>c>>type;--r;--c;}
    cin>>m;
    vector<Pos> humans(m);
    for(auto& [r,c]:humans){cin>>r>>c;--r;--c;}
    bool wall[D][D]={};
    for(int turn=0;turn<300;++turn){
        string cmd(m,'.');
        for(int k=0;k<4;++k){
            auto rot=[&](Pos p){return Pos{k&1?D-1-p.first:p.first,k&2?D-1-p.second:p.second};};
            auto inside=[&](Pos p){auto [r,c]=rot(p);return r<2&&c<2;};
            if(any_of(humans.begin(),humans.end(),inside)||none_of(pets.begin(),pets.end(),inside)) continue;
            vector<Pos> jobs;
            for(Pos p:array<Pos,4>{{{0,2},{1,2},{2,0},{2,1}}}){
                auto q=rot(p);
                if(!wall[q.first][q.second]) jobs.push_back(q);
            }
            if(jobs.empty() || any_of(jobs.begin(),jobs.end(),[&](Pos q){
                return find(humans.begin(),humans.end(),q)!=humans.end()||
                       any_of(pets.begin(),pets.end(),[&](Pos p){return md(p,q)<=1;});
            })) continue;
            array<int,4> guard{};
            function<bool(int,int)> match=[&](int j,int used){
                if(j==(int)jobs.size()) return true;
                for(int i=0;i<m;++i) if(!(used>>i&1)&&md(humans[i],jobs[j])==1){
                    guard[j]=i;
                    if(match(j+1,used|1<<i)) return true;
                }
                return false;
            };
            if(!match(0,0)) continue;
            for(int j=0;j<(int)jobs.size();++j){
                Pos q=jobs[j]; int i=guard[j];
                for(int v=0;v<4;++v) if(add(humans[i],dirs[v])==q) cmd[i]="udlr"[v];
                wall[q.first][q.second]=true;
            }
            break;
        }
        cout<<cmd<<endl;
        for(auto& p:pets){
            string observed; if(!(cin>>observed)) return 1;
            for(char c:observed){
                auto col=string("UDLR").find(c);
                if(col<4) p=add(p,dirs[col]);
            }
            assert(board(p));
        }
    }
}
