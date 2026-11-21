#include <bits/stdc++.h>
#include "../library/shortest-first-masks.hpp"
#include "../problems/ahc008/cat-target-filter.hpp"
using namespace std;
int main(){
 constexpr int R=6,C=7,N=R*C;
 array<array<int,4>,N> graph{};
 for(int r=0;r<R;r++)for(int c=0;c<C;c++)graph[r*C+c]={r?r*C+c-C:-1,r+1<R?r*C+c+C:-1,c?r*C+c-1:-1,c+1<C?r*C+c+1:-1};
 mt19937 rand(98753);
 ShortestFirstMasks<N> bfs;
 CatTargetFilter<N> filter;
 array<float,N> brute{}; float renew=1;
 int source=0;
 array<bool,N> wall{};
 for(int iter=0;iter<200;iter++){
  for(int p=0;p<N;p++) if(p!=source) wall[p]=(rand()%5==0);
  auto ok=[&](int q){return !wall[q];};
  array<int,N> masks{};
  for(int target=0;target<N;target++)if(!wall[target]&&target!=source){
   array<int,N> dist;dist.fill(-1);
   queue<int> q;q.push(target);dist[target]=0;
   while(!q.empty()){
    auto p=q.front();q.pop();
    for(int u:graph[p])if(u>=0&&!wall[u]&&dist[u]<0)dist[u]=dist[p]+1,q.push(u);
   }
   if(dist[source]<=0)continue;
   for(int d=0;d<4;d++){
    int u=graph[source][d];
    if(u>=0&&dist[u]==dist[source]-1)masks[target]|=1<<d;
   }
  }
  bfs.run(source,graph,ok);
  for(int t=0;t<N;t++)assert(bfs.mask[t]==masks[t]);
  int count=count_if(masks.begin(),masks.end(),[](int m){return m>0;});
  if(!count)continue;
  float spill=renew;
  for(int t=0;t<N;t++)if(!masks[t])spill+=brute[t],brute[t]=0;
  array<float,4> probs{};
  for(int t=0;t<N;t++) if(masks[t]){
   brute[t]+=spill/count;
   for(int d=0;d<4;d++)if(masks[t]>>d&1)probs[d]+=brute[t]/popcount(unsigned(masks[t]));
  }
  auto prediction=filter.next(source,graph,ok,bfs);
  for(int d=0;d<4;d++)assert(abs(prediction[d]-probs[d])<1e-5);
  vector<int> dirs;
  for(int d=0;d<4;d++)if(probs[d]>.001)dirs.push_back(d);
  assert(!dirs.empty());
  int d=dirs[rand()%dirs.size()],mid=graph[source][d],finish=source;
  vector<int> dst;
  for(int q:graph[mid])if(q>=0&&!wall[q])dst.push_back(q);
  finish=dst[rand()%dst.size()];
  filter.observe(source,d,mid,finish,graph,ok,bfs);
  for(int t=0;t<N;t++)brute[t]=masks[t]>>d&1 ? brute[t]/popcount(unsigned(masks[t]))/probs[d] : 0;
  renew=brute[mid]+brute[finish]; brute[mid]=brute[finish]=0;
  for(int t=0;t<N;t++)assert(abs(filter.target[t]-brute[t])<1e-5);
  assert(abs(filter.reset-renew)<1e-5);
  source=finish;
 }
 cout<<"OK: 200 random maps and consecutive Bayesian updates\n";
}
