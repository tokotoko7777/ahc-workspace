#pragma once
#include <algorithm>
#include <cassert>
#include <string_view>
#include <utility>
#include <vector>

// Probability that a labelled Markov chain's first 'length' emitted
// characters contain a pattern. The start label is emitted first.
// transition is row-major n*n probabilities. Uses the reachable
// (chain state, matched-prefix length) product and killed-path powering.
inline double markov_pattern_probability(std::string_view labels,
    const std::vector<double>& transition,int start,long long length,std::string_view pattern){
  const int nodes=int(labels.size()),K=int(pattern.size());
  assert(nodes>0&&0<=start&&start<nodes&&int(transition.size())==nodes*nodes);
  if(K==0)return 1.;
  if(length<K)return 0.;
  std::vector<int> sigma(K);
  for(int i=1;i<K;++i){
    int j=sigma[i-1];
    while(j>0&&pattern[j]!=pattern[i])j=sigma[j-1];
    sigma[i]=j+(pattern[j]==pattern[i]);
  }
  auto advance=[&](int k,char a){
    while(k&&pattern[k]!=a)k=sigma[k-1];
    return k+(pattern[k]==a);
  };
  int init=advance(0,labels[start]);
  if(init==K)return 1.;
  using Edge=std::pair<int,double>;
  std::vector<int> idx(nodes*K,-1);
  std::vector<std::pair<int,int>> states{{start,init}};
  std::vector<std::vector<Edge>> graph;
  idx[start*K+init]=0;
  for(int i=0;i<int(states.size());++i){
    auto [s,k]=states[i];
    std::vector<Edge> edges;
    for(int d=0;d<nodes;++d){
      double p=transition[s*nodes+d];
      if(p<=0.)continue;
      int next=advance(k,labels[d]);
      if(next==K)continue;
      int& id=idx[d*K+next];
      if(id<0){id=int(states.size());states.push_back({d,next});}
      edges.emplace_back(id,p);
    }
    graph.push_back(std::move(edges));
  }
  const int n=int(states.size());
  std::vector<double> a(n*n),b(n*n),v(n),w(n);
  for(int i=0;i<n;++i)for(auto [j,p]:graph[i])a[i*n+j]+=p;
  v[0]=1.;
  for(long long t=length-1;t>0;){
    if(t&1){
      std::fill(w.begin(),w.end(),0.);
      for(int i=0;i<n;++i)if(v[i])for(int j=0;j<n;++j)w[j]+=v[i]*a[i*n+j];
      v.swap(w);
    }
    t>>=1;
    if(!t)break;
    std::fill(b.begin(),b.end(),0.);
    for(int i=0;i<n;++i)for(int k=0;k<n;++k)if(a[i*n+k])
      for(int j=0;j<n;++j)b[i*n+j]+=a[i*n+k]*a[k*n+j];
    a.swap(b);
  }
  double avoided=0.;
  for(auto x:v)avoided+=x;
  return std::clamp(1.-avoided,0.,1.);
}
