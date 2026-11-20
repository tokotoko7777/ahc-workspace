#include <bits/stdc++.h>
#include "../../library/hungarian.hpp"
#include "../../library/binary-split-fit.hpp"
#include "../../library/segment-intersection.hpp"
using namespace std;
#ifndef AHC051_FIT
#define AHC051_FIT 1
#endif
namespace {
using Clock=chrono::steady_clock;
using Mass=array<double,20>;
struct Point{long long x,y;};
struct Edge{int a,b;};
struct Node{int a=-1,b=-1,k=-1;};
struct Problem{
  int n,m,k,in,root;
  vector<Point> points;
  vector<Node> g;
  vector<Mass> p;
  vector<int> top,id;
  mt19937 rng{123456789};
  bool crosses(Edge e, Edge f)const{
    if(e.a==f.a||e.a==f.b||e.b==f.a||e.b==f.b)return false;
    return segments_intersect(points[e.a],points[e.b],points[f.a],points[f.b]);
  }
  double distance(int a,int b)const{return hypot(double(points[a].x-points[b].x),double(points[a].y-points[b].y));}
  void read(){
    cin>>n>>m>>k;in=n+m;points.resize(in+1);points[in]={0,5000};p.resize(k);
    for(int i=0;i<in;++i)cin>>points[i].x>>points[i].y;
    for(auto& row:p)for(int i=0;i<n;++i)cin>>row[i];
  }
  void build(const vector<int>& order){
    g.assign(in,{});root=order[0];
    vector<Edge> edges{{in,root}};
    vector<char> used(in,false);
    for(int val:order|views::drop(1)){
      double best=numeric_limits<double>::infinity();int idx=-1,site=-1;
      for(int j=0;j<int(edges.size());++j){
        const auto [a,b]=edges[j];
        if(b>=n)continue;
        for(int x=n;x<in;++x)if(!used[x]){
          const double cost=distance(a,x)+distance(x,b)+distance(x,val)-distance(a,b);
          if(cost>=best)continue;
          const Edge proposed[]={{a,x},{x,b},{x,val}};
          bool legal=true;
          for(int q=0;q<int(edges.size())&&legal;++q)if(q!=j)
            for(auto e:proposed)if(crosses(e,edges[q])){legal=false;break;}
          if(legal){best=cost;idx=j;site=x;}
        }
      }
      if(idx<0) continue;
      auto [a,b]=edges[idx];edges[idx]={a,site};edges.push_back({site,b});edges.push_back({site,val});
      if(a==in)root=site;
      else if(g[a].a==b)g[a].a=site;
      else g[a].b=site;
      g[site]={b,val,0};used[site]=true;
    }
    top.clear();
    auto walk=[&](auto&& self,int v)->void{
      top.push_back(v);
      if(v>=n){self(self,g[v].a);self(self,g[v].b);}
    };
    walk(walk,root);
  }
  void init(){
    vector<unsigned> combined(in);
    for(int v:top|views::reverse){
      if(v<n){combined[v]=1u<<v;continue;}
      auto& [a,b,type]=g[v];
      combined[v]=combined[a]|combined[b];
      double best=-1;
      for(int t=0;t<k;++t){
        double va=0,vb=0;
        for(int c=0;c<n;++c){
          if(combined[a]>>c&1u)va+=p[t][c];
          if(combined[b]>>c&1u)vb+=p[t][c];
        }
        va/=popcount(combined[a]);vb/=popcount(combined[b]);
        if(abs(va-vb)>best){
          best=abs(va-vb);type=t;
        }
      }
      double orientation=0;
      for(int c=0;c<n;++c)
        orientation+=p[type][c]*((combined[a]>>c&1u)?1.0/popcount(combined[a]):(combined[b]>>c&1u)?-1.0/popcount(combined[b]):0.0);
      if(orientation<0)swap(a,b);
    }
  }
  vector<Mass> incoming()const{
    vector<Mass> mass(in);
    mass[root].fill(1);
    for(int v:top)if(v>=n){
      const auto [a,b,t]=g[v];
      for(int c=0;c<n;++c){mass[a][c]=mass[v][c]*p[t][c];mass[b][c]=mass[v][c]*(1-p[t][c]);}
    }
    return mass;
  }
  double match(){
    const auto mass=incoming();
    vector<vector<double>> cost(n,vector<double>(n));
    for(int i=0;i<n;++i)for(int c=0;c<n;++c)cost[i][c]=-mass[i][c];
    id=hungarian_minimum_assignment(cost);
    double result=0;
    for(int i=0;i<n;++i)result+=mass[i][id[i]];
    return result;
  }
  void update(int site){
    // For each kind, probability of finishing in its assigned leaf, starting at
    // each vertex. With upstream mass fixed, one sorter's score is affine in p.
    const auto mass=incoming();
    vector<Mass> success(in);
    for(int v:top|views::reverse){
      if(v<n){success[v][id[v]]=1;continue;}
      const auto [a,b,t]=g[v];
      for(int c=0;c<n;++c)success[v][c]=success[a][c]*p[t][c]+success[b][c]*(1-p[t][c]);
    }
    auto& [a,b,t]=g[site];
    Mass coefficient{};
    for(int c=0;c<n;++c)coefficient[c]=mass[site][c]*(success[a][c]-success[b][c]);
    const auto fit=best_binary_split(p,coefficient);
    t=int(fit.row);if(fit.complement)swap(a,b);
  }
  void run(Clock::time_point limit){
    vector<int> order(n);iota(order.begin(),order.end(),0);
    vector<Node> best_g;vector<int> best_id;int best_root=0;double best=-1,baseline=-1;
    auto keep=[&](double s){if(s>best+1e-12){best=s;best_g=g;best_id=id;best_root=root;}};
    vector<pair<vector<Node>,int>> geometries;
    for(int rep=0;rep<5 && (rep==0||Clock::now()<limit);++rep){
      if(rep==0)sort(order.begin(),order.end(),[&](auto a,auto b){return distance(in,a)<distance(in,b);});
      else shuffle(order.begin(),order.end(),rng);
      build(order);init();double s=match();keep(s);
      geometries.emplace_back(g,root);
    }
    baseline=best;
    if(AHC051_FIT)for(auto& [tree,r]:geometries){
      if(Clock::now()>=limit)break;
      g=tree;root=r;top.clear();
      auto walk=[&](auto&& self,int v)->void{
        top.push_back(v);if(v>=n){self(self,g[v].a);self(self,g[v].b);}
      };walk(walk,root);
      double value=match();
      for(int sweep=0;sweep<8&&Clock::now()<limit;++sweep){
        for(int v:top|views::reverse)if(v>=n)update(v);
        const double s=match();assert(s>=value-1e-10);keep(s);
        if(s<=value+1e-11)break;
        value=s;
      }
    }
    cerr<<"base="<<llround(1e9*(1-baseline/n))<<" final="<<llround(1e9*(1-best/n))<<'\n';
    for(int i=0;i<n;++i)cout<<(i?" ":"")<<best_id[i];
    cout<<'\n'<<best_root<<'\n';
    for(int v=n;v<in;++v){auto [a,b,t]=best_g[v];cout<<t;if(t>=0)cout<<' '<<a<<' '<<b;cout<<'\n';}
  }
};
}
int main(){
  const auto limit=Clock::now()+chrono::milliseconds(1400);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem a;a.read();a.run(limit);
}
