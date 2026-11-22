 // Find pairs of currently staffed irregular gates. A pair is used only
 // if it removes more reachable animals than either of its walls alone.
 // Keep all other baseline commands and certify their simultaneous union.
 optional<string> pairGate(int rem,const string& base){
  const int m=humans.size();
  if(m!=5||turn<160||turn>=245||rem<2||overBudget())return nullopt;
  auto clear=[](Bitboard::Rows& b,int p){b[p/S]&=~(1U<<(p%S));};
  auto test=[](const Bitboard::Rows& b,int p){return (b[p/S]>>(p%S))&1U;};
  Bitboard::Rows board{};
  for(int p=0;p<V;++p)if(!wall[p])board[p/S]|=1U<<(p%S);
  auto post=humans;
  for(int i=0;i<m;++i)if(base[i]!='.'){
   if(base[i]>='a')clear(board,nb[humans[i]][find(BUILD,BUILD+4,base[i])-BUILD]);
   else post[i]=nb[humans[i]][find(MOVE,MOVE+4,base[i])-MOVE];
  }
  const auto before=gateScore(board,post);
  if(!before.together||before.area<302)return nullopt;
  struct Job{int person,q,d,contact;};
  vector<Job> jobs;
  jobs.reserve(4*m);
  for(int i=0;i<m;++i)if(base[i]<'a')for(int d=0;d<4;++d){
   int q=nb[humans[i]][d];
   if(q<0||block[q]||!test(board,q))continue;
   int contact=0;
   for(int dx=-1;dx<=1;++dx)for(int dy=-1;dy<=1;++dy)if(dx||dy){
    int x=q/S+dx,y=q%S+dy;
    contact+=x<0||x>=S||y<0||y>=S||!test(board,pos(x,y));
   }
   if(contact>=2)jobs.push_back({i,q,d,contact});
  }
  const int limit=max(300,(before.area*70+99)/100);
  long long best=before.score;
  int j1=-1,j2=-1;
  for(int a=0;a<int(jobs.size());++a)for(int b=0;b<a;++b){
   const auto x=jobs[a],y=jobs[b];
   if(x.person==y.person||x.q==y.q)continue;
   auto h=post;
   h[x.person]=humans[x.person];h[y.person]=humans[y.person];
   if(find(h.begin(),h.end(),x.q)!=h.end()||find(h.begin(),h.end(),y.q)!=h.end())continue;
   auto xy=board;clear(xy,x.q);clear(xy,y.q);
   const auto r=gateScore(xy,h);
   if(!r.together||r.area<limit||r.score<=best)continue;
   auto p=board;clear(p,x.q);
   const auto u=gateScore(p,h);
   p=board;clear(p,y.q);
   const auto v=gateScore(p,h);
   if(r.score*1LL*u.area<=u.score*1LL*r.area||r.score*1LL*v.area<=v.score*1LL*r.area)continue;
   j1=a;j2=b;best=r.score;
  }
  if(j1<0)return nullopt;
  auto cmd=base;
  for(int j:{j1,j2})cmd[jobs[j].person]=BUILD[jobs[j].d];
  return cmd;
 }
