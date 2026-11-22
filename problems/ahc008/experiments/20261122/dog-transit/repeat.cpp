#pragma GCC optimize("O3")
#include <iostream>
#include <iomanip>
#include "transit.hpp"
using namespace std;
using D=transit::DogEdgeTransit;
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,m,r,c,t;
 if(!(cin>>n))return 0; vector<int> ps(n),ts(n);
 for(int j=0;j<n;j++){cin>>r>>c>>t;ps[j]=(r-1)*30+c-1;ts[j]=t;}
 cin>>m;vector<int> hs(m);
 for(int j=0;j<m;j++){cin>>r>>c;hs[j]=(r-1)*30+c-1;}
 D::Grid wall{};D::PlanState z;
 int count=1,spent=0;
 array<int,9> sum{};
 vector<int> active_types=ts;
 if(argc>1)z.max_turns=atoi(argv[1]);
 for(int turn=0;turn<300;turn++){
  if(z.finished&&z.closed&&count==1&&75-turn>=25){
    auto d=D::bfs(wall,hs[0]);
    bool active=false;
    for(int j=0;j<n;j++)if(ts[j]==4){if(d[ps[j]]==D::INF)active_types[j]=0;else active=true;}
    if(active){
     ++count; spent=turn;
     sum={z.elapsed,z.wall_turns,z.guard_turns,z.all_staffed_turns,z.builds,z.walks,z.waits,z.captured_dogs,z.captured_other};
     z=D::PlanState{};
     z.max_turns=75-turn;
    }
  }

  string act=z.plan(hs,ps,active_types,wall);
  if(act.empty())act=string(m,'.');
  cout<<act<<'\n'<<flush;
  for(int j=0;j<m;j++)if(act[j]!='.'){
   auto ns=D::nbs(hs[j]);
   if(act[j]>='a')wall[ns[string_view(D::BUILDS).find(act[j])]]=1;
   else hs[j]=ns[string_view(D::MOVES).find(act[j])];
  }
  for(int j=0;j<n;j++){
   string s;if(!(cin>>s))return 0;
   for(char letter:s)if(letter!='.')ps[j]=D::nbs(ps[j])[string_view(D::MOVES).find(letter)];
  }
 }
 auto largest=D::bfs(wall,hs[0]);
 int residual=0,res_dogs=0;
 for(int j=0;j<n;j++)if(largest[ps[j]]!=D::INF){++residual;res_dogs+=ts[j]==4;}
 if(getenv("TRANSIT_DIAG"))cerr<<"TRANSIT "<<(z.elapsed+sum[0])<<" "<<(z.wall_turns+sum[1])<<" "<<(z.guard_turns+sum[2])<<" "<<(z.all_staffed_turns+sum[3])<<" "<<(z.builds+sum[4])<<" "<<(z.walks+sum[5])<<" "<<(z.waits+sum[6])<<" "<<(z.captured_dogs+sum[7])<<" "<<(z.captured_other+sum[8])<<" "<<residual<<" "<<res_dogs<<"\n";
}
