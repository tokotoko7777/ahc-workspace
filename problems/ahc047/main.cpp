#include <bits/stdc++.h>
#include "../../library/markov-pattern-probability.hpp"
using namespace std;
#ifndef AHC047_PATHS
#define AHC047_PATHS 1
#endif
#ifndef AHC047_MIX
#define AHC047_MIX 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int N=12;
struct Model{string label;array<array<int,N>,N> row{};};
struct Problem {
  int count,m,L;
  vector<pair<string,int>> word;
  void read(){
    cin>>count>>m>>L;assert(m==N);
    word.resize(count);
    for(auto& [s,v]:word)cin>>s>>v;
    sort(word.begin(),word.end(),[](const auto& a,const auto& b){return a.second>b.second;});
  }
  Model cycle(const string& s)const{
    Model p;
    p.label=s;
    int k=s.size();
    for(int i=k;i<N;++i)p.label+=char('a'+(i-k)%6);
    for(int i=0;i<k;++i)p.row[i][(i+1)%k]=100;
    for(int i=k;i<N;++i)p.row[i][0]=100;
    return p;
  }
  double exact(const Model& p)const{
    vector<double> a;
    for(auto& row:p.row)for(int x:row)a.push_back(.01*x);
    double value=0;
    for(auto& [s,w]:word)value+=w*markov_pattern_probability(p.label,a,0,L,s);
    return value;
  }
  Model weighted()const{
    Model p;
    p.label="abcdefabcdef";
    double w[6][6];
    for(auto& row:w)for(auto& x:row)x=10.;
    for(auto& [s,v]:word)
      for(int j=1;j<int(s.size());++j)w[s[j-1]-'a'][s[j]-'a']+=sqrt(double(v));
    for(int i=0;i<N;++i){
      auto* q=w[i%6];
      double sum=accumulate(q,q+6,0.);
      int left=100;
      for(int j=0;j<N;++j){
        p.row[i][j]=int(50*q[j%6]/sum);
        left-=p.row[i][j];
      }
      for(int j=0;j<left;++j)++p.row[i][(i*3+j)%N];
    }
    return p;
  }
  double proxy(const Model& p)const{
    array<array<double,N>,N> a;
    array<double,N> pi,x;
    pi.fill(1./N);
    for(int i=0;i<N;++i)for(int j=0;j<N;++j)a[i][j]=.01*p.row[i][j];
    for(int step=0;step<18;++step){
      x.fill(0);
      for(int i=0;i<N;++i)for(int j=0;j<N;++j)x[j]+=pi[i]*a[i][j];
      // Laziness keeps periodic chains well behaved; balance unchanged.
      for(int i=0;i<N;++i)pi[i]=.2*pi[i]+.8*x[i];
    }
    array<array<int,N>,6> ids;
    array<int,6> sizes{};
    for(int i=0;i<N;++i){int c=p.label[i]-'a';ids[c][sizes[c]++]=i;}
    double res=0;
    for(auto& [s,w]:word){
      int c=s[0]-'a';
      for(int k=0;k<sizes[c];++k)x[k]=pi[ids[c][k]];
      for(int k=1;k<int(s.size());++k){
        int b=s[k]-'a';array<double,N> y{};
        for(int j=0;j<sizes[c];++j)for(int d=0;d<sizes[b];++d)
          y[d]+=x[j]*a[ids[c][j]][ids[b][d]];
        x=y;c=b;
      }
      double freq=accumulate(x.begin(),x.begin()+sizes[c],0.);
      res+=w*(-expm1(-L*freq));
    }
    return res;
  }
  Model shared_paths(int variant)const{
    Model p;
    p.label="abcdef";
    array<int,6> want{};
    for(int k=0;k<8;++k)for(char c:word[k].first)++want[c-'a'];
    if(variant==0)p.label+="abcdef";
    else{
      array<int,6> used{1,1,1,1,1,1};
      for(int k=6;k<N;++k){
        int b=0;
        for(int a=1;a<6;++a)if(want[a]*used[b]>want[b]*used[a])b=a;
        ++used[b];p.label+=char('a'+b);
      }
    }
    array<array<double,N>,N> mass;
    for(auto& row:mass)row.fill(0.7);
    array<double,N> total;
    total.fill(0.7*N);
    for(int idx=0;idx<10;++idx){
      const auto& [w,weight]=word[idx];
      const double pay=1.5+sqrt(weight)/18.;
      array<array<double,N>,12> v;
      array<array<int,N>,12> parent{};
      for(auto& row:v)row.fill(-1e30);
      for(int j=0;j<N;++j)if(p.label[j]==w[0])v[0][j]=0.;
      for(int k=1;k<int(w.size());++k)
        for(int j=0;j<N;++j)if(p.label[j]==w[k])
          for(int i=0;i<N;++i)if(p.label[i]==w[k-1]){
            double score=v[k-1][i]+log((mass[i][j]+pay)/(total[i]+pay));
            if(score>v[k][j]){v[k][j]=score;parent[k][j]=i;}
          }
      int last=int(w.size())-1;
      int j=max_element(v[last].begin(),v[last].end())-v[last].begin();
      for(int k=last;k>0;--k){
        int i=parent[k][j];
        mass[i][j]+=pay;total[i]+=pay;
        j=i;
      }
    }
    for(int i=0;i<N;++i){
      int left=100;array<pair<double,int>,N> rank;
      for(int j=0;j<N;++j){
        double ideal=100*mass[i][j]/total[i];
        p.row[i][j]=int(ideal);left-=p.row[i][j];
        rank[j]={ideal-p.row[i][j],j};
      }
      sort(rank.rbegin(),rank.rend());
      for(int j=0;j<left;++j)++p.row[i][rank[j].second];
    }
    return p;
  }
  Model improve(Model incumbent,Clock::time_point deadline)const{
    double actual=exact(incumbent);
    mt19937 rng(470447);
    for(int pass=0;pass<(AHC047_PATHS?6:4);++pass){
      if(Clock::now()>=deadline)break;
      Model p;
      if(pass<4)p=pass==3?weighted():cycle(word[pass].first);
      else if(AHC047_PATHS==1)p=shared_paths(pass-4);
      else{
        p=weighted();
        shuffle(p.label.begin(),p.label.end(),rng);
      }
      if(pass<3){
        for(auto& row:p.row){
          auto i=max_element(row.begin(),row.end())-row.begin();
          row.fill(2);row[i]+=76;
        }
      }
      double value=proxy(p),record=value;
      Model peak=p;
      for(int j=0;j<4500;++j){
        if((j&63)==0&&Clock::now()>=deadline)break;
        int i=rng()%N,a=rng()%N,b=rng()%N;
        bool letters=(rng()%100)<2;
        if(letters){
          if(a==b||p.label[a]==p.label[b])continue;
          swap(p.label[a],p.label[b]);
        }else{
          if(a==b||!p.row[i][a])continue;
          int amount=min(p.row[i][a],1+int(rng()%20));
          p.row[i][a]-=amount;p.row[i][b]+=amount;
          double t=1-j/4500.,temp=1000*t*t+4;
          double z=proxy(p);
          if(z>=value||double(rng())/4294967296.<exp((z-value)/temp)){
            value=z;if(z>record){record=z;peak=p;}
          }else{p.row[i][a]+=amount;p.row[i][b]-=amount;}
          continue;
        }
        double t=1-j/4500.,temp=1000*t*t+4;
        double z=proxy(p);
        if(z>=value||double(rng())/4294967296.<exp((z-value)/temp)){
          value=z;if(z>record){record=z;peak=p;}
        }else swap(p.label[a],p.label[b]);
      }
      double score=exact(peak);
      if(score>actual){incumbent=peak;actual=score;}
    }
    return incumbent;
  }
};
}
#ifndef AHC047_TEST
int main(){
  const auto end=Clock::now()+chrono::milliseconds(1500);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();
  auto q=p.cycle(p.word[0].first);
  const auto base=p.exact(q);
  if(AHC047_MIX)q=p.improve(q,end);
  cerr<<"base="<<llround(base)<<" score="<<llround(p.exact(q))<<'\n';
  for(int i=0;i<N;++i){
    cout<<q.label[i];
    for(int w:q.row[i])cout<<' '<<w;
    cout<<'\n';
  }
}
#endif
