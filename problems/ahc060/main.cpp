#include <bits/stdc++.h>
#include "../../library/nonbacktracking-path.hpp"
using namespace std;
#ifndef AHC060_FRONTIER
#define AHC060_FRONTIER 1
#endif
#ifndef AHC060_RICH
#define AHC060_RICH 2
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int N=100,K=10,DEPTH=20,WORK=1000;
struct Path{
  int prev=-1,at=0,length=0,flip=-1,flip_after=-1;
  int cost()const{return length+(flip>=0);}
  uint64_t word=1;
  array<uint8_t,DEPTH> move{};
  uint32_t priority=0;
};
struct Solver{
  array<vector<int>,N> g;
  array<bool,N> red{},want{};
  array<int,N> dist{},visits{},delivered{};
  array<unordered_set<string>,K> stock;
  mt19937 random{1496007};
  int t,prev=-1,at=0,score=0,dispatch=0;
  long long probes=0;
  string cone;
  vector<int> answer;
  void read(){
    int n,m,k;cin>>n>>m>>k>>t;assert(n==N&&k==K);
    for(int a,b;m--;){cin>>a>>b;g[a].push_back(b);g[b].push_back(a);}
    for(int i=K;i<N;++i)want[i]=random()%2;
    queue<int> q;dist.fill(N);
    for(int i=0;i<K;++i){dist[i]=0;q.push(i);}
    while(!q.empty()){
      int v=q.front();q.pop();
      for(int u:g[v])if(dist[u]>dist[v]+1){dist[u]=dist[v]+1;q.push(u);}
    }
  }
  void move_to(int v,bool warm){
    assert(v!=prev&&find(g[at].begin(),g[at].end(),v)!=g[at].end()&&int(answer.size())<t);
    answer.push_back(v);prev=at;at=v;++visits[v];
    if(v<K){
      if(stock[v].insert(cone).second)++score;
      cone.clear();++delivered[v];
    }else{
      cone.push_back(red[v]?'R':'W');
      if(warm&&want[v]&&!red[v]&&int(answer.size())<t){
        red[v]=true;answer.push_back(-1);
      }
    }
  }
  void warmup(){
    while(int(answer.size())<t&&(int(answer.size())<360||at>=K)){
      int v=-1,best=INT_MIN;
      for(int u:g[at])if(u!=prev){
        int x=180*int(!visits[u])-9*visits[u]+int(random()%100);
        if(x>best){best=x;v=u;}
      }
      move_to(v,true);
    }
  }
  static string unpack(uint64_t bits){
    string result;
    while(bits>1){result.push_back(bits%2?'R':'W');bits>>=1;}
    ranges::reverse(result);
    return result;
  }
  Path choose(int steps,bool frontier,Clock::time_point deadline){
    const int cap=AHC060_RICH?1800:WORK, width=AHC060_RICH?104:56;
    const bool can_flip=AHC060_RICH==2&&dispatch%12==0;
    Path best,spare;
    best.length=spare.length=DEPTH+1;
    int quality=INT_MIN,backup=INT_MIN,used=0;
    Path initial;initial.prev=prev;initial.at=at;assert(cone.empty());
    auto advance=[&](const Path& p,int u){
      ++used;
      auto a=p;a.prev=p.at;a.at=u;a.move[a.length++]=uint8_t(u);
      if(u>=K)a.word=2*a.word+(red[u]||u==a.flip);
      return a;
    };
    auto submit=[&](const Path& p){
      if(p.at>=K)return;
      const int balance=9*int(delivered[p.at]==0)-int(stock[p.at].size());
      const int s=1400-45*p.cost()-20*(p.flip>=0)+balance+int(random()%11);
      if(p.flip<0&&s>backup){spare=p;backup=s;}
      if(!stock[p.at].contains(unpack(p.word))&&s>quality){best=p;quality=s;}
    };
    if(frontier){
      vector<Path> level{initial};
      for(int d=0;d<steps&&!level.empty()&&used<cap&&Clock::now()<deadline;++d){
        vector<Path> next;
        unordered_set<uint64_t> seen;
        seen.reserve(level.size()*4);
        auto add=[&](Path a,int u){
          const uint64_t key=(((a.word*N+a.prev)*N+u)*(N+1)+a.flip+1);
          if(!seen.insert(key).second)return;
          a.priority=(uint32_t(N-dist[u])<<12)+(random()&8191u);
          next.push_back(a);
        };
        for(const auto& p:level){
          for(int u:g[p.at])if(u!=p.prev&&used<cap&&p.cost()<steps){
            auto a=advance(p,u);
            if(u<K){submit(a);continue;}
            add(a,u);
            if(can_flip&&a.flip<0&&!red[u]&&a.cost()+2<=steps&&used<cap){
              a.flip=u;a.flip_after=a.length-1;
              ++used;add(a,u);
            }
          }
        }
        if(best.length<DEPTH+1&&d>=best.cost())break;
        if(next.size()>size_t(width)){
          nth_element(next.begin(),next.begin()+width,next.end(),
                      [](const auto& a,const auto& b){return a.priority>b.priority;});
          next.resize(width);
        }
        level=move(next);
      }
    }else{
      while(used<cap&&Clock::now()<deadline){
        auto p=initial;
        for(int k=0;k<steps&&used<cap;++k){
          const int count=int(g[p.at].size())-(p.prev>=0);
          int idx=random()%count;
          int u=-1;
          for(int j:g[p.at])if(j!=p.prev&&idx--==0){u=j;break;}
          p=advance(p,u);
          if(u<K){submit(p);break;}
        }
      }
    }
    probes+=used;
    if(best.length<=DEPTH)return best;
    if(spare.length<=DEPTH)return spare;
    // Start already is a shop: find a nonempty return/transfer by trying
    // all legal first steps and selecting the shortest continuation.
    vector<int> route;
    for(int u:g[at])if(u!=prev){
      auto p=shortest_nonbacktracking_path(g,at,u,[](int v){return v<K;});
      if(p&&(route.empty()||p->size()+1<route.size())){
        route={u};
        route.insert(route.end(),p->begin(),p->end());
      }
    }
    assert(!route.empty());
    Path out;out.length=min<int>(steps,route.size());
    for(int i=0;i<out.length;++i)out.move[i]=route[i];
    return out;
  }
  void run(Clock::time_point cutoff){
    warmup();
    const int initial=score;
    while(int(answer.size())<t&&Clock::now()<cutoff){
      assert(cone.empty());
      const int steps=min(DEPTH,t-int(answer.size()));
      const auto p=choose(steps,AHC060_FRONTIER,cutoff);
      if(p.length==0)break;
      for(int i=0;i<p.length;++i){
        move_to(p.move[i],false);
        if(i==p.flip_after){
          assert(at==p.flip&&at>=K&&!red[at]&&int(answer.size())<t);
          red[at]=true;answer.push_back(-1);
        }
      }
      ++dispatch;
      if(at>=K)break;
    }
    cerr<<"base="<<initial<<" score="<<score<<" probes="<<probes<<'\n';
    for(int op:answer)cout<<op<<'\n';
  }
};
}
int main(){
  const auto cutoff=Clock::now()+chrono::milliseconds(1450);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Solver p;p.read();p.run(cutoff);
}
