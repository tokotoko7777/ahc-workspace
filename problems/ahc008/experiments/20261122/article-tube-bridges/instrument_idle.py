import sys
from pathlib import Path
s=Path(sys.argv[1]).read_text()
key=' array<int,300> integratedMode{},integratedReady{},integratedLocks{};'
assert s.count(key)==1
s=s.replace(key,key+'''
 struct BridgeAudit{int t,w,b,from,to,supply,full;};
 vector<BridgeAudit> bridgeAudit;
 void auditUnusedBridge(const vector<int>& ready,bitset<V> used,const bitset<V>& build){
  if(turn<101||turn>152)return;
  const bool full=all_of(ready.begin(),ready.end(),[](int q){return q!=0;});
  auto lane=[&](int p){return bankRot?p/S:p%S;};
  bool occ[V]{};
  for(int p:humans)occ[p]=true;
  for(Pet p:pets)occ[p.p]=true;
  int population[30]{};
  for(int p:humans)++population[lane(p)];
  for(int i=0;i<(int)banks.size();i++){
   const auto& x=banks[i];
   if(!ready[i]||wall[x.g]||wall[x.k]||build[x.g]||build[x.k]||occ[x.g]||occ[x.k])continue;
   if(any_of(x.inner.begin(),x.inner.end(),[&](int p){return wall[p]||build[p]||occ[p];}))continue;
   for(auto [f,e]:{pair{x.l,x.h},pair{x.h,x.l}}){
    if(population[lane(e)]||population[lane(f)]<2)continue;
    int supply=0;
    for(int q=0;q<(int)banks.size();q++)if(q!=i){
     const auto& y=banks[q];
     if(!ready[q]||wall[y.g]||wall[y.k]||build[y.g]||build[y.k]||
         any_of(y.inner.begin(),y.inner.end(),[&](int p){return wall[p]||build[p];}))continue;
     supply+=((lane(y.l)==lane(e)&&md(y.l,e)<=3)||(lane(y.h)==lane(e)&&md(y.h,e)<=3));
    }
    if(!supply)continue;
    for(int j=0;j<(int)humans.size();j++)if(!used[j]&&act[j]=='.'&&humans[j]==f)
     bridgeAudit.push_back({turn,j,i,f,e,supply,full});
   }
  }
 }
''',1)
key='  if(scheduled&&!partial)parkUnassigned(ready,paths,usedPeople,build,moveTo,service);'
assert s.count(key)==1
s=s.replace(key,'''  if(scheduled&&!partial){
   auditUnusedBridge(ready,usedPeople,build);
   parkUnassigned(ready,paths,usedPeople,build,moveTo,service);
  }''',1)
key=' if(const char* p=getenv("AHC008_GLOBAL_CLEAR_LOG")){'
assert s.count(key)==1
s=s.replace(key,''' if(const char* p=getenv("AHC008_BRIDGE_IDLE_LOG")){
  ofstream f(p);
  for(const auto& a:g.bridgeAudit)
   f<<a.t<<" "<<a.w<<" "<<a.b<<" "<<a.from<<" "<<a.to<<" "<<a.supply<<" "<<a.full<<'\\n';
 }
'''+key,1)
Path(sys.argv[2]).write_text(s)
