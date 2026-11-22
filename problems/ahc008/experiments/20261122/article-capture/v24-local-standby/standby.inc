 bool localStandbyEnabled=true;
 int localStandbyMatched=0,localStandbyMoves=0;
 void localStandby(const vector<int>& ready,bitset<V>& used,
     const bitset<V>& build,bitset<V>& moveTo){
  if(!localStandbyEnabled||humans.size()!=5)return;
  bitset<V> closed=build;
  for(int p=0;p<V;p++)if(wall[p])closed[p]=true;
  array<int,V> plain;plain.fill(-1);
  const auto column=[&](int p){return bankRot?p/S:p%S;};
  const auto row=[&](int p){return bankRot?p%S:p/S;};
  const int axis=bankRot?1:S;
  for(int road:terryRoads){
   int c=column(road);
   vector<int> staff;
   for(int i=0;i<5;i++)if(!used[i]&&column(humans[i])==c)staff.push_back(i);
   if(staff.empty())continue;
   // A guard already here never changes aisle under this local standby
   // rule. Staff assigned by any earlier phase were excluded above.
   for(int i:staff)used[i]=true;
   struct Choice {int gain=0,go=-1;};
   int m=staff.size(),N=pets.size();
   vector<vector<Choice>> choice(m,vector<Choice>(N));
   for(int j=0;j<m;j++){
    int from=humans[staff[j]];
    BankPath nav;nav.run(from,closed,plain);
    for(int k=0;k<int(banks.size());k++){
     const auto& x=banks[k];
     if(!ready[k]||crossing.reserves(k)||terryRoad.bridge[k]||closed[x.g]||closed[x.k]||
        any_of(x.inner.begin(),x.inner.end(),[&](int p){return closed[p];}))continue;
     int lane=column(x.l)==c?x.l:(column(x.h)==c?x.h:-1);
     if(lane<0)continue;
     const int f=row(from),g=row(lane),sd=abs(f-g);int step=(g>f?axis:-axis);
     bool clear=true;
     for(int p=from;p!=lane;) {p+=step;if(closed[p]){clear=false;break;}}
     if(!clear)continue;
     for(int pi=0;pi<N;pi++){
      const auto& p=pets[pi];int d=nav.d[p.p];
      if(d<0||d>15)continue; // actual reachable nearby animal
      int pd=min(md(p.p,x.inner[1]),md(p.p,x.inner[2]));
      if(pd>8)continue;
      int gain=1000000/(20+6*sd+8*pd+d);
      if(gain>choice[j][pi].gain)choice[j][pi]={gain,sd?from+step:-1};
     }
    }
   }
   // Independent min-cost matching inside each aisle; the same animal
   // remains eligible on the opposite aisle to help pair prealignment.
   int L=1<<m;
   vector<int> dp(L,-1),next;
   vector<vector<int>> pick(N,vector<int>(L,-1));
   dp[0]=0;
   for(int pi=0;pi<N;pi++){
    next=dp;
    for(int mask=0;mask<L;mask++)if(dp[mask]>=0)for(int j=0;j<m;j++)
     if(!(mask>>j&1)&&choice[j][pi].gain>0){
      int q=mask|1<<j,v=dp[mask]+choice[j][pi].gain;
      if(v>next[q])next[q]=v,pick[pi][q]=j;
     }
    dp.swap(next);
   }
   int mask=max_element(dp.begin(),dp.end())-dp.begin();
   for(int pi=N-1;pi>=0&&mask;pi--)if(int j=pick[pi][mask];j>=0){
    mask^=1<<j;
    int i=staff[j],q=choice[j][pi].go;++localStandbyMatched;
    if(q>=0&&!closed[q]){
     act[i]=MOVE[dir(humans[i],q)];moveTo[q]=true;++localStandbyMoves;
    }
   }
  }
 }
