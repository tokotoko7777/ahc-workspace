 // During the rescue, lease at most two walkers to one active rail. The
 // other three commands and any existing wall assignments are preserved.
 int mixedTube=-1,mixedA=-1,mixedB=-1,mixedBegan=-1,mixedUntil=0;
 optional<string> mixedRail(int rem,const vector<int>& ready,const vector<int>& enabled,const vector<int>& count,const string& base){
  const int m=humans.size();
  if(m!=5||turn<160||turn>=225||rem<2||overBudget())return nullopt;
  bitset<V> build;
  vector<int> future=humans;
  for(int i=0;i<m;++i)if(base[i]!='.'){
   if(base[i]>='a')build[nb[humans[i]][find(BUILD,BUILD+4,base[i])-BUILD]]=1;
   else future[i]=nb[humans[i]][find(MOVE,MOVE+4,base[i])-MOVE];
  }
  bitset<V> unavailable=build;
  array<int,V> body;body.fill(-1);
  for(int q=0;q<V;q++)unavailable[q]=unavailable[q]||wall[q];
  for(int k=0;k<int(banks.size());++k)if(ready[k])for(int p:banks[k].inner)body[p]=k;
  array<BankPath,10> nav;
  for(int i=0;i<m;++i)if(base[i]<'a')nav[i].run(humans[i],unavailable,body);
  struct P{int a,b,d;};
  vector<vector<P>> avail(banks.size());
  int horizon=0;
  int bestK=-1,bestA=-1,bestB=-1;
  double best=0;
  auto legalPair=[&](int k,int a,int b){
   const auto& x=banks[k];
   for(int i=0;i<m;++i)if(i!=a&&i!=b)
    if(future[i]==x.g||future[i]==x.k||find(x.inner.begin(),x.inner.end(),future[i])!=x.inner.end())return false;
   return true;
  };
  for(int k=0;k<int(banks.size());++k){
   if(!ready[k]||!enabled[k]||terryRoad.bridge[k]||crossing.reserves(k))continue;
   const auto& x=banks[k];
   if(unavailable[x.g]||unavailable[x.k])continue;
   for(int a=0;a<m;++a)if(base[a]<'a'&&nav[a].d[x.l]>=0)
    for(int b=0;b<m;++b)if(a!=b&&base[b]<'a'&&nav[b].d[x.h]>=0){
     int eta=max(nav[a].d[x.l],nav[b].d[x.h]);
     if(eta==0&&count[k]&&!block[x.g]&&!block[x.k]&&legalPair(k,a,b)){
      auto cmd=base;
      cmd[a]=BUILD[dir(x.l,x.g)];cmd[b]=BUILD[dir(x.h,x.k)];
      // The two added walls must retain all workers after existing builds.
      auto post=future;post[a]=humans[a];post[b]=humans[b];
      Bitboard::Rows rows{};
      for(int p=0;p<V;++p)if(!unavailable[p]&&p!=x.g&&p!=x.k)rows[p/S]|=1U<<(p%S);
      auto v=gateScore(rows,post);
      if(v.together&&v.area>=300){mixedTube=-1;mixedUntil=turn+1;return cmd;}
     }
     if(eta>5||!legalPair(k,a,b)||turn<mixedUntil)continue;
     if(mixedTube>=0&&(k!=mixedTube||a!=mixedA||b!=mixedB))continue;
     avail[k].push_back({a,b,max(1,eta)});horizon=max(horizon,max(1,eta));
    }
  }
  if(!horizon||turn-mixedBegan>7&&mixedTube>=0){
   if(mixedTube>=0){mixedTube=-1;mixedUntil=turn+2;}
   return nullopt;
  }
  if(!postReady)observedMotionGraph();
  const auto prior=mcDogs();
  const auto sim=terry_mc::sample_paths(mcState(),terrySim,32,horizon,&prior);
  for(int k=0;k<int(banks.size());++k)if(!avail[k].empty()){
   const auto& x=banks[k];
   array<double,6> eval,prob;eval.fill(-1.);
   for(auto p:avail[k]){
    if(eval[p.d]<0){
     auto s=terry_mc::capture_window(sim,terrySim,x.g,x.k,x.inner,p.d);
     eval[p.d]=s.expected;prob[p.d]=s.any;
    }
    if(prob[p.d]<(mixedTube==k?.125:.25))continue;
    double v=eval[p.d]/(2+p.d);
    if(v>best){best=v;bestK=k;bestA=p.a;bestB=p.b;}
   }
  }
  if(bestK<0){
   if(mixedTube>=0){mixedTube=-1;mixedUntil=turn+2;}
   return nullopt;
  }
  const auto& x=banks[bestK];
  string cmd=base;
  for(auto [i,g]:{pair{bestA,x.l},pair{bestB,x.h}}){
   int q=nav[i].step[g];
   if(q>=0&&q!=humans[i])cmd[i]=MOVE[dir(humans[i],q)];
   else cmd[i]='.';
  }
  if(mixedTube<0)mixedBegan=turn;
  mixedTube=bestK;mixedA=bestA;mixedB=bestB;
  if(cmd==base)return nullopt;
  return cmd;
 }
