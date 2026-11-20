#include <cassert>
#include <random>
#include <set>
#include "fixed_walk_relocation.hpp"
using namespace std;
using G=vector<vector<int>>;
using V=vector<int>;
struct Walk { V moves, flip; };
static Walk split(const G& g,int k,const V& a){
  Walk out{{},V(g.size(),-1)};
  int v=0,p=-1;
  for(int x:a) {
    if(x==-1) {
      assert(v>=k && out.flip[v]<0 && !out.moves.empty());
      out.flip[v]=int(out.moves.size())-1;
    } else {
      assert(x!=p && find(g[v].begin(),g[v].end(),x)!=g[v].end());
      out.moves.push_back(x);p=v;v=x;
    }
  }
  return out;
}
static int replay(int k,const Walk& w) {
  vector<set<string>> seen(k); string cone;
  int score=0;
  for(int i=0;i<int(w.moves.size());++i) {
    int v=w.moves[i];
    if(v<k){ score+=seen[v].insert(cone).second; cone.clear(); }
    else cone+= w.flip[v]>=0 && w.flip[v]<i?'R':'W';
  }
  return score;
}
static Walk brute(int k,Walk w) {
  for(int v=k;v<int(w.flip.size());++v) if(w.flip[v]>=0) {
    int best=w.flip[v],val=replay(k,w);
    for(int i=0;i<int(w.moves.size());++i) if(w.moves[i]==v) {
      w.flip[v]=i;
      int q=replay(k,w);
      if(q>val){val=q;best=i;}
    }
    w.flip[v]=best;
  }
  return w;
}
static G cycle(int n) {
  G g(n);for(int i=0;i<n;++i)g[i]={(i+1)%n,(i+n-1)%n};return g;
}
static int check(const G& g,int k,const V& a) {
  auto old=split(g,k,a);
  const auto got=ahc060::relocate(g,k,a.size(),a);
  auto cur=split(g,k,got.actions),o=brute(k,old);
  assert(a.size()==got.actions.size() && old.moves==cur.moves);
  for(int v=k;v<int(g.size());++v)assert((old.flip[v]<0)==(cur.flip[v]<0));
  assert(replay(k,old)==got.before && replay(k,cur)==got.after);
  assert(got.after>=got.before && replay(k,o)==got.after && cur.flip==o.flip);
  assert(ahc060::relocate(g,k,a.size(),a,0).actions==a);
  return got.after;
}
int main() {
  // 0 -> 1 delivers empty. Shop 0 receives WW twice and RR twice.
  // Advancing either flip releases one duplicate to a third distinct word.
  const V collision{1,2,3,0,1,2,-1,3,-1,0,1,2,3,0,1,2,3,0};
  assert(replay(2,split(cycle(4),2,collision))==3);
  assert(check(cycle(4),2,collision)==4);
  auto reject=[](const V& ops,int limit){
    try { (void)ahc060::relocate(cycle(4),2,limit,ops); }
    catch(const invalid_argument&) { return; }
    assert(false);
  };
  reject({-1},1);                       // No departed tree.
  reject({1,2,-1,-1},4);               // No second flip.
  reject({1,2,-1,1},4);                // Flip does not reset backtracking.
  reject({1,2,-1},2);                  // Destination exists, budget does not.
  G g=cycle(89); V a;
  for(int i=1;i<=4*89+20;++i){a.push_back(i%89);if(i==100||i==101)a.push_back(-1);}
  check(g,1,a);
  mt19937 rng(0x6ff06021);
  for(int t=0;t<2000;++t){
    int n=4+rng()%17,k=1+rng()%(n-1);g=cycle(n);
    for(int i=0;i<n*2;++i){
      int v=rng()%n,u=rng()%n;
      if(v!=u&&find(g[v].begin(),g[v].end(),u)==g[v].end()){g[v].push_back(u);g[u].push_back(v);}
    }
    const int len=15+rng()%190;
    a.clear();V red(n); int at=0,prev=-1;
    for(int i=0;i<len;++i){
      V cand;for(int u:g[at])if(u!=prev)cand.push_back(u);
      int v=cand[rng()%cand.size()];a.push_back(v);prev=at;at=v;
      if(v>=k&&!red[v]&&rng()%5==0){a.push_back(-1);red[v]=1;}
    }
    check(g,k,a);
  }
}
