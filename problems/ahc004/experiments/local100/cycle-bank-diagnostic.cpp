#include <bits/stdc++.h>
#include "observed-cycles.hpp"
using namespace std;
int main(int argc,char** argv){
 for(int f=1;f<argc;++f){
  ifstream in(argv[f]);int n,m;in>>n>>m;if(!in)return 1;
  map<string,int> counts;int lo=99,hi=0;
  for(int j=0;j<m;++j){string s;in>>s;++counts[s];lo=min(lo,int(s.size()));hi=max(hi,int(s.size()));}
  int L=(lo+hi)/2;vector<string> reads;vector<int> weights;
  for(auto& [s,k]:counts)reads.push_back(s),weights.push_back(k);
  auto c=ahc004::observed_cycles(n,reads,weights,20000);
  vector<char> seen(reads.size());int u=0,t20=0,t30=0;
  for(int i=0;i<int(c.cycles.size());++i){
   for(int r:c.cycles[i].words)if(!seen[r]) { seen[r]=1; u+=weights[r]; }
   if(i<20)t20=u;if(i<30)t30=u;
  }
  cout<<filesystem::path(argv[f]).filename().string()<<','<<L<<','<<m<<','<<c.cycles.size()<<','<<u<<','<<t20<<','<<t30<<','<<c.capped<<'\n';
 }
}
