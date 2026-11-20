#include <bits/stdc++.h>
using namespace std;
#ifndef AHC030_PROPAGATE
#define AHC030_PROPAGATE 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int Limit=400;
struct Placement{bitset<Limit> mask;vector<int> cells;bool active=true;};
struct Shape{
  vector<Placement> pos;
  array<int,Limit> cover{};
  int active_count=0;
  void filter(int cell,bool contains){
    for(auto& p:pos)if(p.active&&p.mask[cell]!=contains){
      p.active=false;--active_count;
      for(int hit:p.cells)--cover[hit];
    }
    assert(active_count>0);
  }
};
struct Solver{
  int n,m,L,total=0,spent=0;
  double eps;
  vector<Shape> shapes;
  array<int,Limit> observed,lower{},upper{};
  void read(){
    cin>>n>>m>>eps;L=n*n;
    observed.fill(-1);
    shapes.resize(m);
    for(auto& s:shapes){
      int size;cin>>size;total+=size;
      vector<pair<int,int>> offset(size);
      int h=0,w=0;
      for(auto& [r,c]:offset){cin>>r>>c;h=max(h,r);w=max(w,c);}
      for(int y=0;y+h<n;++y)for(int x=0;x+w<n;++x){
        Placement p;
        for(auto [r,c]:offset){
          int i=(y+r)*n+x+c;
          p.mask[i]=1;p.cells.push_back(i);++s.cover[i];
        }
        s.pos.push_back(std::move(p));
      }
      s.active_count=int(s.pos.size());
    }
  }
  int dig(int i){
    cout<<"q 1 "<<i/n<<' '<<i%n<<endl;
    int v;if(!(cin>>v)||v<0)exit(0);
    assert(observed[i]<0);
    ++spent;
    return observed[i]=v;
  }
  bool answer(const vector<int>& cells){
    cout<<"a "<<cells.size();
    for(int c:cells)cout<<' '<<c/n<<' '<<c%n;
    cout<<endl;
    int val;if(!(cin>>val))exit(0);
    return val==1;
  }
  void propagate(Clock::time_point deadline){
    bool changed;
    do{
      if(Clock::now()>=deadline)return;
      changed=false;
      for(int i=0;i<L;++i)if(observed[i]>=0){
        int lo=0,hi=0;
        for(auto& s:shapes){lo+=s.cover[i]==s.active_count;hi+=s.cover[i]>0;}
        assert(lo<=observed[i]&&observed[i]<=hi);
        if(lo==hi)continue;
        if(observed[i]==lo||observed[i]==hi){
          for(auto& s:shapes)if(s.cover[i]>0&&s.cover[i]<s.active_count)
            s.filter(i,observed[i]==hi);
          changed=true;
        }
      }
    }while(changed);
    for(int i=0;i<L;++i){
      int lo=0,hi=0;
      for(auto& s:shapes){lo+=s.cover[i]==s.active_count;hi+=s.cover[i]>0;}
      lower[i]=observed[i]<0?lo:observed[i];
      upper[i]=observed[i]<0?hi:observed[i];
    }
  }
  int next_dig()const{
    int best=-1;
    double best_score=-1;
    for(int i=0;i<L;++i)if(observed[i]<0&&lower[i]<upper[i]){
      double score=0;
      for(auto& s:shapes){
        double p=double(s.cover[i])/s.active_count;
        score+=p*(1-p);
      }
      // A support-ambiguous cell is immediately useful even if it does not
      // distinguish placements as well as a cell already known to contain oil.
      if(lower[i]==0)score*=1.18;
      if(score>best_score){best_score=score;best=i;}
    }
    return best;
  }
  void finish_scan(){
    vector<int> found;
    int accumulated=0;
    for(int i=0;i<L;++i)if(observed[i]>0){found.push_back(i);accumulated+=observed[i];}
    for(int i=0;i<L&&accumulated<total;++i)if(observed[i]<0){
      if(int v=dig(i);v>0){found.push_back(i);accumulated+=v;}
    }
    if(!answer(found))throw logic_error("scan must recover the true support");
  }
  void solve(bool active,Clock::time_point deadline){
    if(!active){finish_scan();return;}
    for(int i=0;i<L;++i){
      propagate(deadline);
      if(Clock::now()>=deadline)break;
      int lo_total=accumulate(lower.begin(),lower.begin()+L,0);
      int hi_total=accumulate(upper.begin(),upper.begin()+L,0);
      assert(lo_total<=total&&total<=hi_total);
      if(lo_total==total||hi_total==total){
        const auto& exact=lo_total==total?lower:upper;
        vector<int> found;
        for(int c=0;c<L;++c)if(exact[c]>0)found.push_back(c);
        if(!answer(found))throw logic_error("mass equality must recover the support");
        return;
      }
      bool known=true;
      vector<int> found;
      for(int c=0;c<L;++c){
        if(lower[c]>0)found.push_back(c);
        else if(upper[c]>0)known=false;
      }
      if(known){
        if(!answer(found))throw logic_error("support is certified");
        return;
      }
      int p=next_dig();
      assert(p>=0);
      dig(p);
    }
    finish_scan();
  }
};
}
#ifndef AHC030_TEST
int main(){
  const auto deadline=Clock::now()+chrono::milliseconds(2300);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Solver s;s.read();s.solve(AHC030_PROPAGATE,deadline);
}
#endif
