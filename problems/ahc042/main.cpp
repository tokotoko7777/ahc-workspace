#include <bits/stdc++.h>
#include "../../library/packed-line-shift.hpp"
using namespace std;
#ifndef AHC042_PREP
#define AHC042_PREP 0
#endif
#ifndef AHC042_WIDTH
#define AHC042_WIDTH 22
#endif
#ifndef AHC042_BEAM
#define AHC042_BEAM 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int N=20,ONI=40;
using Packed=PackedLineShift<N>;
struct Board {
  array<uint32_t,N> x{},o{};
  bool operator==(const Board&)const=default;
};
struct Macro{
  uint8_t axis=0,sign=0,line=0,out=0,back=0;
};
using Plan=vector<Macro>;
struct Choice{Macro prep{},eject{};};
Board apply(const Board& b,Macro m){
  Board c=b;
  auto f=[&](auto& rows){
    uint32_t initial=Packed::get(rows,m.axis,m.line),out=Packed::shifted(initial,m.out,m.sign);
    Packed::put(rows,m.axis,m.line,Packed::shifted(out,m.back,!m.sign));
  };
  f(c.x);f(c.o);
  return c;
}
template<class Visit>
void moves(const Board& b,bool partial,Visit visit){
  for(int axis=0;axis<2;++axis)for(int i=0;i<N;++i){
    uint32_t x=Packed::get(b.x,axis,i),o=Packed::get(b.o,axis,i);
    if(!x)continue;
    for(int positive=0;positive<2;++positive){
      int maxd=o==0?N:positive?N-int(bit_width(o)):int(countr_zero(o));
      int removed=0;
      for(int d=1;d<=maxd;++d){
        int bit=positive?N-d:d-1;
        if(!(x>>bit&1u))continue;
        ++removed;
        Macro m{uint8_t(axis),uint8_t(positive),uint8_t(i),uint8_t(d),uint8_t(d)};
        visit(m,removed);
        if(partial){
          m.back=0;visit(m,removed);
          if(d>1){m.back=d/2;visit(m,removed);}
        }
      }
    }
  }
}
template<class Visit>
void preps(const Board& b,Visit visit){
  int count=0;
  for(int axis=0;axis<2;++axis)for(int i=0;i<N;++i){
    uint32_t x=Packed::get(b.x,axis,i),o=Packed::get(b.o,axis,i);
    for(int sign=0;sign<2;++sign)for(int d=2;d<=7;++d){
      const int k=sign?N-d:d-1;
      if(!(x>>k&1u))continue;
      const uint32_t removed=sign?Packed::mask^((1u<<k)-1u):(1u<<(k+1))-1u;
      const uint32_t blockers=o&removed;
      if(popcount(blockers)!=1)continue;
      const int line_id=countr_zero(blockers);
      for(int t=0;t<2;++t){
        uint32_t at_exit=1u<<(t?N-1:0);
        if((Packed::get(b.x,!axis,line_id)|Packed::get(b.o,!axis,line_id))&at_exit)continue;
        const Macro prep{uint8_t(!axis),uint8_t(t),uint8_t(line_id),1,0};
        auto p=apply(b,prep);
        uint32_t newo=Packed::get(p.o,axis,i),newx=Packed::get(p.x,axis,i);
        if(newo&removed || !(newx&removed))continue;
        const int kills=popcount(newx&removed);
        for(int back:{0,d/2,d}){
          const Macro eject{uint8_t(axis),uint8_t(sign),uint8_t(i),uint8_t(d),uint8_t(back)};
          visit(Choice{prep,eject},kills,apply(p,eject));
        }
        if(++count>=16)return;
      }
    }
  }
}
int projection(const Board& b){
  array<int,N> col_min,col_max;
  for(int j=0;j<N;++j){
    uint32_t o=Packed::get(b.o,1,j);
    col_min[j]=o?countr_zero(o):N;
    col_max[j]=int(bit_width(o))-1;
  }
  int sum=0;
  for(int i=0;i<N;++i){
    uint32_t o=b.o[i];
    int l=o?countr_zero(o):N,r=int(bit_width(o))-1;
    for(uint32_t s=b.x[i];s;s&=s-1){
      int j=countr_zero(s);
      int best=N+1;
      if(j<l)best=min(best,j+1);
      if(j>r)best=min(best,N-j);
      if(i<col_min[j])best=min(best,i+1);
      if(i>col_max[j])best=min(best,N-i);
      if(best>N)return -1;
      sum+=best;
    }
  }
  return sum;
}
Plan greedy(Board b){
  Plan out;
  int remain=ONI;
  while(remain){
    Macro next{};int covered=0;
    moves(b,false,[&](Macro m,int k){
      if(!covered||m.out*covered<next.out*k||(m.out*covered==next.out*k&&m.out<next.out)){
        next=m;covered=k;
      }
    });
    assert(covered>0);
    b=apply(b,next);out.push_back(next);remain-=covered;
  }
  return out;
}
int total(const Plan& p){int s=0;for(auto m:p)s+=m.out+m.back;return s;}
Plan beam(const Board& board,Plan best,Clock::time_point deadline,int& checks){
  struct State{Board board;array<Choice,ONI> seq;int length=0,spent=0,rank=0;};
  array<vector<State>,ONI+1> bucket;
  bucket[ONI].push_back({board,{}});
  int limit=total(best);
  constexpr int Width=AHC042_WIDTH;
  auto ranked=[](const State& a,const State& b){return pair{a.rank,a.spent}<pair{b.rank,b.spent};};
  for(int remain=ONI;remain>0;--remain){
    auto& vec=bucket[remain];
    sort(vec.begin(),vec.end(),ranked);
    vector<const State*> used;
    for(const auto& state:vec){
      if(Clock::now()>=deadline)return best;
      if(int(used.size())>=Width)break;
      if(any_of(used.begin(),used.end(),[&](auto q){return q->board==state.board;}))continue;
      used.push_back(&state);
      auto enter=[&](Choice m,int removed,const Board& next){
        const int cost=state.spent+m.prep.out+m.eject.out+m.eject.back,left=remain-removed;
        ++checks;
        if(cost>=limit)return;
        if(!left){
          auto append=[&](Choice c){if(c.prep.out)best.push_back(c.prep);best.push_back(c.eject);};
          limit=cost;best.clear();
          for(int i=0;i<state.length;++i)append(state.seq[i]);
          append(m);return;
        }
        int p=projection(next);
        if(p<0)return; // Keep a certified restore-only completion.
        State q{next,state.seq,state.length+1,cost,8*cost+9*p};
        q.seq[state.length]=m;
        auto& v=bucket[left];
        v.push_back(q);
        if(v.size()>=24*Width){
          nth_element(v.begin(),v.begin()+12*Width,v.end(),ranked);
          v.resize(12*Width);
        }
      };
      moves(state.board,true,[&](Macro m,int removed){enter(Choice{{},m},removed,apply(state.board,m));});
      if(AHC042_PREP)preps(state.board,enter);
    }
    vector<State>().swap(vec);
  }
  return best;
}
}
#ifndef AHC042_TEST
int main(){
  const auto deadline=Clock::now()+chrono::milliseconds(1450);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  int n;if(!(cin>>n))return 0;
  assert(n==N);
  Board b;
  for(int r=0;r<N;++r){
    string s;cin>>s;
    for(int c=0;c<N;++c){
      if(s[c]=='x')b.x[r]|=1u<<c;
      if(s[c]=='o')b.o[r]|=1u<<c;
    }
  }
  auto plan=greedy(b);int base=total(plan);
  int checks=0;
  if(AHC042_BEAM)plan=beam(b,std::move(plan),deadline,checks);
  cerr<<"base="<<base<<" turns="<<total(plan)<<" checks="<<checks<<'\n';
  for(auto m:plan){
    for(int k=0;k<m.out;++k)cout<<(m.axis?(m.sign?'D':'U'):(m.sign?'R':'L'))<<' '<<int(m.line)<<'\n';
    for(int k=0;k<m.back;++k)cout<<(m.axis?(m.sign?'U':'D'):(m.sign?'L':'R'))<<' '<<int(m.line)<<'\n';
  }
}
#endif
