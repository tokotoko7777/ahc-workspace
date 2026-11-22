 // Find pairs of currently staffed irregular gates. A pair is used only
 // if it removes more reachable animals than either of its walls alone.
 // Keep all other baseline commands and certify their simultaneous union.
 int pairStageAt=-2;
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
  if(j1<0){
   // No already staffed pair. Move at most one worker by one step;
   // initially all pets must be too far from both gates to interfere
   // on the next turn, even along arbitrary legal movement.
   if(pairStageAt==turn-1)return nullopt;
   const int ready=jobs.size();
   struct Approach{int person,stance,gate;};
   vector<Approach> moves;
   array<int,V> noApproach{};
   auto unsafe=[&](int q){
    int& flag=noApproach[q];
    if(flag==0){
     flag=1;
     for(auto a:pets)if(md(a.p,q)<=(a.t==1?1:a.t==3?3:2)+1){flag=2;break;}
    }
    return flag==2;
   };
   if(ready==0)return nullopt;
   for(int i=0;i<m;++i)if(base[i]<'a')for(int st:nb[humans[i]])if(st>=0&&test(board,st))
    for(int g:nb[st])if(g>=0&&g!=humans[i]&&test(board,g)&&!block[g]&&!unsafe(g)){
     int contacts=0;
     for(int x=-1;x<=1;++x)for(int y=-1;y<=1;++y)if(x||y){
      int r=g/S+x,c=g%S+y;
      contacts+=r<0||r>=S||c<0||c>=S||!test(board,pos(r,c));
     }
     if(contacts>=2)moves.push_back({i,st,g});
    }
   int i0=-1, j0=-1;
   for(int i=0;i<int(moves.size());++i)for(int j=0;j<ready;++j){
    const auto a=moves[i];const auto b=jobs[j];
    if(a.person==b.person||a.gate==b.q||a.stance==b.q||unsafe(b.q))continue;
    auto h=post;
    h[a.person]=a.stance;h[b.person]=humans[b.person];
    if(find(h.begin(),h.end(),a.gate)!=h.end()||find(h.begin(),h.end(),b.q)!=h.end())continue;
    auto xy=board;clear(xy,a.gate);clear(xy,b.q);
    const auto r=gateScore(xy,h);
    if(!r.together||r.area<limit||r.score<=best)continue;
    auto p=board;clear(p,a.gate);const auto u=gateScore(p,h);
    p=board;clear(p,b.q);const auto v=gateScore(p,h);
    if(r.score*u.area<=u.score*r.area||r.score*v.area<=v.score*r.area)continue;
    i0=i;j0=j;best=r.score;
   }
   if(i0<0)return nullopt;
   auto cmd=base;
   const auto a=moves[i0];const auto b=jobs[j0];
   cmd[a.person]=MOVE[dir(humans[a.person],a.stance)];
   cmd[b.person]='.';
   if(cmd==base)return nullopt;
   pairStageAt=turn;
   return cmd;
  }
  auto cmd=base;
  for(int j:{j1,j2})cmd[jobs[j].person]=BUILD[jobs[j].d];
  return cmd;
 }
