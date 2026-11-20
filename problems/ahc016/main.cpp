#include <bits/stdc++.h>
#include "../../library/bernoulli-count-distribution.hpp"
using namespace std;
#ifndef AHC016_DEGREES
#define AHC016_DEGREES 1
#endif

namespace {
struct Codebook {
  int n=0;
  vector<string> code;
  virtual int decode(const string&) const=0;
  virtual ~Codebook()=default;
};

int choose_n(int m,double p) {
  double best=-1e100;
  int chosen=100;
  for(int n=4;n<=100;++n){
    int all=n*(n-1)/2;
    if(all<m-1)continue;
    double gap=(1-2*p)*all/(m-1);
    double sigma=sqrt(all*p*(1-p));
    double error=erfc(gap/(2*sqrt(2.0)*sigma))*(m-1.0)/m;
    double expected=100*log1p(-0.1*error)-log(n);
    if(expected>best)chosen=n,best=expected;
  }
  return chosen;
}

struct EdgeCode : Codebook {
  double p;
  vector<int> level;
  EdgeCode(int m,double e):p(e) {
    n=choose_n(m,p);
    const int all=n*(n-1)/2;
    for(int i=0;i<m;++i){
      int k=(all*i+(m-1)/2)/(m-1);
      level.push_back(k);
      code.push_back(string(k,'1')+string(all-k,'0'));
    }
  }
  int decode(const string& s) const override {
    double observed=count(s.begin(),s.end(),'1'), best=1e100;
    int answer=0;
    for(int i=0;i<int(level.size());++i){
      double v=abs(observed-(s.size()*p+(1-2*p)*level[i]));
      if(v<best)best=v,answer=i;
    }
    return answer;
  }
};

struct ExactCode : Codebook {
  vector<int> owner;
  explicit ExactCode(int m) {
    for(n=4;n<=6;++n){
      int all=n*(n-1)/2;
      int num[6][6]{};
      int k=0;
      for(int i=0;i<n;++i)for(int j=i+1;j<n;++j) num[i][j]=num[j][i]=k++;
      vector<int> perm(n);
      iota(perm.begin(),perm.end(),0);
      vector<array<unsigned,15>> maps;
      do {
        array<unsigned,15> mapped{};
        for(int i=0;i<n;++i)for(int j=i+1;j<n;++j)
          mapped[num[i][j]]=1u<<num[perm[i]][perm[j]];
        maps.push_back(mapped);
      }while(next_permutation(perm.begin(),perm.end()));
      owner.assign(1u<<all,-1);
      code.clear();
      for(unsigned g=0;g<owner.size() && int(code.size())<m;++g){
        if(owner[g]>=0)continue;
        for(const auto& mapping:maps){
          unsigned h=0;
          for(unsigned subset=g;subset;subset&=subset-1) h|=mapping[countr_zero(subset)];
          owner[h]=int(code.size());
        }
        string s(all,'0');
        for(int i=0;i<all;++i) if(g>>i&1) s[i]='1';
        code.push_back(std::move(s));
      }
      if(int(code.size())==m) return;
    }
    assert(false);
  }
  int decode(const string& s) const override {
    unsigned mask=0;
    for(int i=0;i<int(s.size());++i)if(s[i]=='1')mask|=1u<<i;
    assert(owner[mask]>=0);
    return owner[mask];
  }
};

using Feature=array<double,6>;
Feature feature(const vector<double>& hist,int n) {
  // Uniform five rank bands; take fractional mass at the cut to avoid jumpy quantiles.
  Feature f{};
  double at=0,step=n/5.0;
  for(int d=0;d<int(hist.size());++d){
    double next=at+hist[d];
    f[0]+=d*hist[d]/n;
    for(int b=0;b<5;++b){
      double share=max(0.0,min(next,(b+1)*step)-max(at,b*step));
      f[b+1]+=d*share/step;
    }
    at=next;
  }
  for(int b=1;b<=5;++b) f[b]-=f[0];
  f[0]*=1.55;
  return f;
}

double distance(const Feature& a,const Feature& b) {
  double out=0;
  for(int i=0;i<6;++i){double x=a[i]-b[i];out+=x*x;}
  return out;
}

struct Candidate {
  array<int,3> size;
  int mask;
  Feature f;
};

struct DegreeCode : Codebook {
  vector<Feature> selected;
  int pair_id[3][3]={{0,1,2},{1,3,4},{2,4,5}};
  bool connected(int mask,int a,int b) const {return (mask>>pair_id[a][b])&1;}
  DegreeCode(int m,double p) {
    n=choose_n(m,p);
    vector<vector<double>> pmf(n);
    for(int d=0;d<n;++d){
      vector<double> events(n-1,p);
      fill_n(events.begin(),d,1-p);
      pmf[d]=bernoulli_count_distribution(events);
    }
    vector<Candidate> options;
    constexpr int patterns[]={0b001111,0b001001,0b000010,
                              0b110000,0b110110,0b111101};
    vector<double> hist(n);
    for(int a=0;a<=n;++a)for(int b=0;b<=n-a;++b)for(int pat:patterns){
      array<int,3> size{a,b,n-a-b};
      fill(hist.begin(),hist.end(),0.0);
      for(int u=0;u<3;++u)if(size[u]){
        int d=0;
        for(int v=0;v<3;++v)if(connected(pat,u,v)) d+=size[v]-(u==v);
        for(int i=0;i<n;++i) hist[i]+=size[u]*pmf[d][i];
      }
      options.push_back({size,pat,feature(hist,n)});
    }
    vector<double> nearest(options.size(),1e100);
    for(int i=0;i<m;++i) {
      int pick=i==0?0:int(max_element(nearest.begin(),nearest.end())-nearest.begin());
      const auto& c=options[pick];
      selected.push_back(c.f);
      vector<int> group;
      group.reserve(n);
      for(int x=0;x<3;++x)for(int y=0;y<c.size[x];++y)group.push_back(x);
      string s;
      s.reserve(n*(n-1)/2);
      for(int u=0;u<n;++u)for(int v=u+1;v<n;++v)
        s.push_back('0'+connected(c.mask,group[u],group[v]));
      code.push_back(std::move(s));
      for(int j=0;j<int(options.size());++j)
        nearest[j]=min(nearest[j],distance(c.f,options[j].f));
      nearest[pick]=-1;
    }
  }
  int decode(const string& s) const override {
    vector<int> degree(n);
    int i=0;
    for(int u=0;u<n;++u)for(int v=u+1;v<n;++v,++i)
      if(s[i]=='1')++degree[u],++degree[v];
    vector<double> hist(n);
    for(int d:degree)++hist[d];
    auto f=feature(hist,n);
    double best=1e100;
    int answer=0;
    for(int j=0;j<int(selected.size());++j){
      double d=distance(f,selected[j]);
      if(d<best)best=d,answer=j;
    }
    return answer;
  }
};
} // namespace

#ifndef AHC016_NO_MAIN
int main(){
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int m;
  double p;
  if(!(cin>>m>>p))return 0;
  unique_ptr<Codebook> code;
  if(p==0)code=make_unique<ExactCode>(m);
  else if(AHC016_DEGREES) code=make_unique<DegreeCode>(m,p);
  else code=make_unique<EdgeCode>(m,p);
  cout<<code->n<<'\n';
  for(const auto& s:code->code)cout<<s<<'\n';
  cout.flush();
  for(int i=0;i<100;++i){
    string s;
    if(!(cin>>s))return 0;
    cout<<code->decode(s)<<endl;
  }
}
#endif
