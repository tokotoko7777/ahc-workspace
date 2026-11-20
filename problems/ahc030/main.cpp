#include <bits/stdc++.h>
using namespace std;
#ifndef AHC030_SURVEY
#define AHC030_SURVEY 0
#endif
#ifndef AHC030_PROPAGATE
#define AHC030_PROPAGATE 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int Limit=400;
struct Placement{bitset<Limit> mask;vector<int> cells;bool active=true;double weight=1;};
struct Shape{
  vector<Placement> pos;
  array<int,Limit> cover{};
  int active_count=0;
  void weighted_coverage(array<double,Limit>& hit,double& sum)const{
    hit.fill(0.);sum=0.;
    for(const auto& p:pos)if(p.active){
      sum+=p.weight;
      for(int c:p.cells)hit[c]+=p.weight;
    }
  }
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
  int next_dig(bool weighted)const{
    vector<array<double,Limit>> probability;
    if(weighted){
      probability.resize(m);
      for(int k=0;k<m;++k){
        double z=0.;
        shapes[k].weighted_coverage(probability[k],z);
        for(int i=0;i<L;++i)probability[k][i]/=z;
      }
    }
    int best=-1;
    double best_score=-1;
    for(int i=0;i<L;++i)if(observed[i]<0&&lower[i]<upper[i]){
      double score=0;
      for(int k=0;k<m;++k){
        const auto& s=shapes[k];
        double p=weighted?probability[k][i]:double(s.cover[i])/s.active_count;
        score+=p*(1-p);
      }
      // A support-ambiguous cell is immediately useful even if it does not
      // distinguish placements as well as a cell already known to contain oil.
      if(lower[i]==0)score*=1.18;
      if(score>best_score){best_score=score;best=i;}
    }
    return best;
  }
  using Mask=bitset<Limit>;
  struct Moment {double mean=0,var=0;};
  Moment moments(const Shape& s,const Mask& mask)const{
    double z=0.,one=0.,two=0.;
    for(auto& p:s.pos)if(p.active){
      int v=int((p.mask&mask).count());
      z+=p.weight;one+=p.weight*v;two+=p.weight*v*v;
    }
    const double mu=one/z;
    return {mu,max(0.,two/z-mu*mu)};
  }
  void survey_prior(Clock::time_point deadline){
    mt19937 rng(30030);
    vector<Mask> previous;
    const double scale=1-2*eps;
    for(int t=0;t<14&&Clock::now()<deadline;++t){
      Mask best;
      double best_info=0.;
      vector<Moment> saved;
      for(int j=0;j<34;++j){
        int h=2+rng()%max(1,3*n/4-1),w=2+rng()%max(1,3*n/4-1);
        if(j%7==0)h=n;
        if(j%7==1)w=n;
        int y=rng()%(n-h+1),x=rng()%(n-w+1);
        Mask q;
        for(int r=y;r<y+h;++r)for(int c=x;c<x+w;++c)q[r*n+c]=1;
        if(find(previous.begin(),previous.end(),q)!=previous.end())continue;
        double v=0.;
        vector<Moment> stats;
        for(auto& s:shapes){stats.push_back(moments(s,q));v+=stats.back().var;}
        const int k=int(q.count());
        double info=log1p(scale*scale*v/(k*eps*(1-eps)+1.0/12))*sqrt(k);
        if(info>best_info){best_info=info;best=q;saved=std::move(stats);}
      }
      if(best_info==0.)break;
      previous.push_back(best);
      const int k=int(best.count());
      cout<<"q "<<k;
      for(int i=0;i<L;++i)if(best[i])cout<<' '<<i/n<<' '<<i%n;
      cout<<endl;
      int response;
      if(!(cin>>response)||response<0)exit(0);
      double mean=0.,var=0.;
      for(auto [f,v]:saved){mean+=f;var+=v;}
      for(int i=0;i<m;++i){
        const auto [f,v]=saved[i];
        const double sd=sqrt(k*eps*(1-eps)+scale*scale*max(0.,var-v)+1.0/12);
        vector<double> log_likelihood(L+1,numeric_limits<double>::quiet_NaN());
        auto& shape=shapes[i];
        double peak=-1e100;
        for(auto& p:shape.pos)if(p.active){
          int hit=int((p.mask&best).count());
          if(isnan(log_likelihood[hit])){
            double mu=k*eps+scale*(mean-f+hit);
            double z=(response-mu)/sd;
            // Clip observation: response=0 includes the entire left tail.
            log_likelihood[hit]=response==0?
              log(max(1e-32,0.5*erfc((mu-0.5)/(sd*sqrt(2.))))):-0.5*z*z;
          }
          p.weight=log(p.weight)+0.55*log_likelihood[hit];
          peak=max(peak,p.weight);
        }
        // Soft prior changes selection only, never removes candidates.
        for(auto& p:shape.pos)if(p.active)
          p.weight=exp(max(-7.,p.weight-peak));
      }
    }
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
    if(AHC030_SURVEY)survey_prior(min(deadline,Clock::now()+chrono::milliseconds(700)));
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
      int p=next_dig(AHC030_SURVEY);
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
