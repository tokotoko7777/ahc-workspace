#pragma GCC optimize("O3")
#include "../../wild-geometry.hpp"
#include <chrono>
#include <iostream>
using namespace std;
static int score_board(vector<string> a,const vector<string>& words) {
    for(int c=0;c<20;++c){string col;for(int r=0;r<20;++r)col+=a[r][c];a.push_back(std::move(col));}
    for(auto& s:a)s+=s.substr(0,19);
    int hit=0;for(const auto& w:words)
      hit+=any_of(a.begin(),a.end(),[&](const string& s){return s.find(w)<20;});
    return hit;
}
int main(){
    ios::sync_with_stdio(false);
    int m,limit,width;cin>>m>>limit>>width;
    vector<string>w(m);for(auto& s:w)cin>>s;
    const auto begin=chrono::steady_clock::now();
    auto past=[&](double ms){return chrono::duration<double,milli>(chrono::steady_clock::now()-begin).count()>ms;};
    auto b=ahc004::make_wild_bank(ahc004::partial_assembled_lines(20,w,4),12);
    auto a=ahc004::wild_triples(b,width,1368000,[&] {return past(limit*.35);});
    auto f=ahc004::wild_fourth(b,a,15000000,[&] {return past(limit*.83);});
    cerr<<b.n<<' '<<a.examined<<' '<<a.seeds.size()<<' '<<f.tested<<' '<<f.parents<<' '<<f.ranked.size()<<' '
        <<chrono::duration<double,milli>(chrono::steady_clock::now()-begin).count()<<'\n';
    int k=0; std::set<std::array<int,4>> seen;
    for(const auto& q:f.ranked) {
        if(past(limit)||k>=32)break;
        if(!seen.insert(ahc004::wild_anchor_key(q)).second)continue;
        auto r=ahc004::wild_realize(b,q,[&] {return past(limit);});
        if(r.board.empty())continue; ++k;
        cout<<score_board(r.board,w)<<' '<<r.rows<<' '<<r.columns<<' '<<q.strong<<' '<<q.weak<<' '<<r.extensions<<'\n';
        for(const auto& x:r.board)cout<<x<<'\n';
    }
}
