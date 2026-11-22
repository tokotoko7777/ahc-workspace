 // Find single-wall separators in the actual movement graph in one DFS.
 // Each option must keep all workers on a large connected exterior; its
 // assigned worker routes entirely inside that certified exterior.
 int spineTarget=-1,spineWorker=-1,spineUntil=-1,spineSteps=0,spineClosed=0;
 optional<string> spineCut(int rem,const string* base=nullptr){
  const bool merge=base!=nullptr;
  const int m=humans.size(),root=group[humans.front()];
  if(m!=5||turn<160||turn>=245||rem<2||currentCt[root]<302||overBudget())return nullopt;
  for(int h:humans)if(group[h]!=root)return nullopt;
  int tick=0;
  array<int,V> in{},low{},out{};
  array<array<int,4>,V> sub{};
  array<int,V> degree{};
  // At the occupied DFS root a wall cannot be built, so its child cuts
  // need not appear. Group each separating subtree by its wall vertex.
  auto traverse=[&](auto&& self,int p,int par)->void{
   in[p]=low[p]=++tick;
   for(int q:nb[p])if(q>=0&&!wall[q]&&q!=par){
    if(in[q])low[p]=min(low[p],in[q]);
    else{
     self(self,q,p);
     if(low[q]>=in[p]&&par>=0)sub[p][degree[p]++]=q;
     low[p]=min(low[p],low[q]);
    }
   }
   out[p]=tick;
  };
  traverse(traverse,humans.front(),-1);
  struct Option{double gain=0;int p=-1,w=-1,q=-1,d=-1;};
  Option best;
  for(int p=0;p<V;p++)if(degree[p]){
   auto trapped=[&](int q){
    if(q==p)return true;
    if(!in[q])return false;
    for(int child:span(sub[p]).first(degree[p]))if(in[child]<=in[q]&&in[q]<=out[child])return true;
    return false;
   };
   if(any_of(humans.begin(),humans.end(),trapped))continue;
   int kept=tick-1;
   for(int child:span(sub[p]).first(degree[p]))kept-=out[child]-in[child]+1;
   if(kept<300||100*kept<70*tick)continue;
   int hit=0,closest=100;
   for(auto a:pets)if(a.p!=p&&trapped(a.p)){
    ++hit;closest=min(closest,md(a.p,p));
   }
   if(!hit)continue;
   // Real routes in the retained side. Depth four is sufficient; a
   // longer detour is too uncertain to replace an ordinary rescue move.
   array<int,V> d{},first{};
   int queue[V],len=0;
   for(int q:nb[p])if(q>=0&&in[q]&&!trapped(q)){
    d[q]=1;first[q]=q;queue[len++]=q;
   }
   for(int j=0;j<len;j++){
    int q=queue[j];if(d[q]>=5)continue;
    for(int v:nb[q])if(v>=0&&in[v]&&!d[v]&&!trapped(v)){
     d[v]=d[q]+1;first[v]=q;queue[len++]=v;
    }
   }
   for(int i=0;i<m;i++)if(d[humans[i]]){
    int dist=d[humans[i]]-1;
    if(merge&&((*base)[i]>='a'||(dist>0&&(*base)[i]!='.')))continue;
#ifdef AHC008_SPINE_IMMEDIATE
    if(dist!=0)continue;
#endif
    if(dist==0&&block[p])continue;
    // Do not spend a turn chasing an animal about to leave at the mouth.
    if(dist>0&&(closest<=dist||block[p]||(turn>=spineUntil&&p==spineTarget)))continue;
    int next=first[humans[i]];
    double v=(ldexp(double(kept),min(4,hit))-tick)/(1.+1.65*dist);
    if(p==spineTarget&&i==spineWorker&&turn<spineUntil)v*=1.30;
    if(v>best.gain)best={v,p,i,next,dist};
   }
  }
  if(best.p<0){spineTarget=spineWorker=-1;spineUntil=-1;return nullopt;}
  if(merge){
   auto cmd=*base;
   auto clear=[](Bitboard::Rows& b,int p){b[p/S]&=~(1U<<(p%S));};
   auto filled=[](const Bitboard::Rows& b,int p){return (b[p/S]>>(p%S))&1U;};
   Bitboard::Rows rows{};
   for(int p=0;p<V;++p)if(!wall[p])rows[p/S]|=1U<<(p%S);
   auto future=humans;
   for(int i=0;i<m;++i)if((*base)[i]!='.'){
    const char c=(*base)[i];
    if(c>='a')clear(rows,nb[humans[i]][find(BUILD,BUILD+4,c)-BUILD]);
    else future[i]=nb[humans[i]][find(MOVE,MOVE+4,c)-MOVE];
   }
   const auto status=gateScore(rows,future);
   if(!filled(rows,best.p))return nullopt;
   future[best.w]=best.d?best.q:humans[best.w];
   for(int h:future)if(!filled(rows,h)||h==best.p)return nullopt;
   if(best.d==0){
    clear(rows,best.p);
    const auto seal=gateScore(rows,future);
    if(!seal.together||seal.area<300||seal.area*100<70*tick||seal.score<=status.score)return nullopt;
    cmd[best.w]=BUILD[dir(humans[best.w],best.p)];++spineClosed;
    spineTarget=spineWorker=-1;spineUntil=-1;
   }else{
    cmd[best.w]=MOVE[dir(humans[best.w],best.q)];++spineSteps;
    if(best.p!=spineTarget||best.w!=spineWorker)spineUntil=turn+best.d+1;
    spineTarget=best.p;spineWorker=best.w;
   }
   return cmd;
  }
  if(best.d==0){
   ++spineClosed;
   string cmd(m,'.');
   cmd[best.w]=BUILD[dir(humans[best.w],best.p)];
   spineTarget=spineWorker=-1;spineUntil=-1;
   return cmd;
  }
  if(best.p!=spineTarget||best.w!=spineWorker)spineUntil=turn+best.d+1;
  spineTarget=best.p;spineWorker=best.w;++spineSteps;
  string cmd(m,'.');cmd[best.w]=MOVE[dir(humans[best.w],best.q)];
  return cmd;
 }
