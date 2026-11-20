#include <bits/stdc++.h>
#include "../../library/markov-pattern-probability.hpp"
using namespace std;
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
  Model improve(Model incumbent,Clock::time_point deadline)const{
    double actual=exact(incumbent);
    mt19937 rng(470447);
    for(int pass=0;pass<4;++pass){
      if(Clock::now()>=deadline)break;
      Model p=pass==3?weighted():cycle(word[pass].first);
      if(pass!=3){
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
