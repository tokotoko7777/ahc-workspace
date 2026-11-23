#define main pinned_reference_entry
#include "pinned.private.cpp"
#undef main
#include <random>
#include <numeric>
#include <tuple>
namespace T=terry_exact_maps;
namespace E=exact_build;
namespace M=terry_exact_motion;
constexpr std::array<int,8> gates{215,335,455,155,575,695,824,815};
using std::vector;
using std::array;
constexpr int V=900, N=256, INF=10000000;
E::Layout layout(const T::Terrain& t){
 E::Layout x;
 for(int p=0;p<V;++p)x.ascii[p]=char(t.cells[p]);
 x.aisleRows=t.aisleRows;x.aisleCols=t.aisleColumns;return x;
}
T::Terrain augmented(int map,const T::Terrain& t){
 auto q=t;
 if(map!=3)return q;
 auto nominal=T::classify(t,t.plannedWalls,{}, {t.aisleColumns.front()});
 for(int g:gates){
  assert(t.doorCells[g]&&!t.aisle(g)&&!t.planned(g));
  int id=nominal.ids[g];assert(0<=id&&id<nominal.count());
  const auto r=nominal.rooms[id];assert(r.left>=0&&r.right>=0&&r.size==5);
  int opposite=r.left==g?r.right:r.left;
  assert((r.left==g||r.right==g)&&t.doorCells[opposite]&&!t.aisle(opposite));
  assert(g%30!=29&&opposite%30!=29);
  if(r.left==g){
   assert(t.cells[opposite]==T::Marker::RightTwo);q.cells[opposite]=T::Marker::RightOne;
  }else{
   assert(t.cells[opposite]==T::Marker::LeftTwo);q.cells[opposite]=T::Marker::LeftOne;
  }
  q.cells[g]=T::Marker::Wall;q.plannedWalls[g]=true;q.doorCells[g]=false;
 }
 assert(q.plannedWalls.count()==241);
 return q;
}
struct Cap {int gate,stance,dir;};
array<Cap,8> caps(const T::Terrain& t){
 array<Cap,8> z{};
 for(size_t i=0;i<gates.size();++i){
  int p=gates[i];assert(t.leftDoor(p)!=t.rightDoor(p));
  int s=t.leftDoor(p)?p-1:p+1, d=t.leftDoor(p)?1:3;
  assert(t.aisle(s)&&E::nb(s)[d]==p);
  z[i]={p,s,d};
 }
 return z;
}
void append(E::Crew& c,const T::Terrain& orig,const T::Terrain& mod,const vector<int>& starts){
 const auto marks=caps(orig);
 E::Board w{},ww{};
 for(int p=0;p<V;++p){w[p]=orig.planned(p);ww[p]=mod.planned(p);}
 array<E::BFS,8> transit;
 for(int a=0;a<8;++a)transit[a].run(marks[a].stance,ww);
 const int m=starts.size();
 struct Options{
  array<int,N> cost,last;
  array<array<int,8>,N> parent{};
 };
 vector<Options> pass(m);
 array<int,N> dp,next;dp.fill(INF);dp[0]=0;
 vector<array<int,N>> assigned(m);
 for(int id=0;id<m;++id){
  auto& opt=pass[id];opt.cost.fill(INF);opt.last.fill(-1);
  int p=starts[id],t=0;
  for(auto job:c.q[id])if(job.mask){
   // Stock queue is unchanged. Estimate its finish with physical original
   // nominal walls, counting original duplicate jobs conservatively.
   E::BFS b;b.run(p,w);
   assert(b.d[job.at]<E::INF);t+=b.d[job.at]+std::popcount(job.mask);p=job.at;
  }
  opt.cost[0]=t;
  E::BFS b;b.run(p,ww);
  array<array<int,8>,N> d;
  for(auto& x:d)x.fill(INF);
  for(int a=0;a<8;++a)d[1<<a][a]=t+b.d[marks[a].stance]+1;
  for(int q=1;q<N;++q)for(int a=0;a<8;++a)if(q>>a&1&&d[q][a]<INF){
   if(d[q][a]<opt.cost[q])opt.cost[q]=d[q][a],opt.last[q]=a;
   for(int k=0;k<8;++k)if(!(q>>k&1)){
    int q2=q|1<<k,cost=d[q][a]+transit[a].d[marks[k].stance]+1;
    if(cost<d[q2][k])d[q2][k]=cost,opt.parent[q2][k]=a;
   }
  }
  next.fill(INF);assigned[id].fill(-1);
  for(int q=0;q<N;++q)if(dp[q]<INF){
   int a=(N-1)^q;
   for(int add=a;;add=(add-1)&a){
    int q2=q|add,cost=std::max(dp[q],opt.cost[add]);
    if(cost<next[q2])next[q2]=cost,assigned[id][q2]=add;
    if(add==0)break;
   }
  }
  dp=next;
 }
 assert(dp.back()<INF);
 int q=N-1;
 for(int id=m-1;id>=0;--id){
  int sub=assigned[id][q];assert(sub>=0);
  q^=sub;int a=pass[id].last[sub];vector<int> seq;
  while(sub){
   assert(a>=0&&((sub>>a)&1));
   seq.push_back(a);
   int prev=pass[id].parent[sub][a];sub^=1<<a;a=prev;
  }
  for(auto it=seq.rbegin();it!=seq.rend();++it){
   const auto j=marks[*it];c.q[id].push_back({j.stance,1u<<j.dir});
  }
 }
 assert(!q);
 // Install additional nominal goals only after preserving/assigning all
 // original routed tasks unchanged. Downstream matcher sees actual caps.
 c.layout=layout(mod);
}
struct Splices {int count=0, extra=0, min_extra=0, without=0;};
Splices splice(E::Crew& crew,const T::Terrain& orig,const T::Terrain& target,const vector<int>& start){
 auto stock=crew;
 auto fallback=crew;append(fallback,orig,target,start);
 E::Board walls{};
 for(int p=0;p<V;++p)walls[p]=target.planned(p);
 const auto known=caps(orig);
 vector<E::BFS> paths(V);
 vector<bool> done(V);
 auto dist=[&](int a,int b){
  if(!done[a]){paths[a].run(a,walls);done[a]=true;}
  return paths[a].d[b];
 };
 vector<int> finish(start.size());
 for(int i=0;i<int(start.size());++i){
  int p=start[i];
  for(auto j:crew.q[i]){
   if(!j.mask)continue;
   int d=dist(p,j.at);assert(d<E::INF);finish[i]+=d+std::popcount(j.mask);p=j.at;
  }
 }
 std::bitset<V> chosen;
 Splices out;
 for(const auto& cap:known){
  // Stable cap order and all actual entry/exit legs. Each splice is on an
  // extant travel leg; first starts are physical starting locations.
  auto best=std::tuple{E::INF,E::INF,E::INF,E::INF};
  int crew_id=-1,position=-1,cost=0,min_relax=3;
  for(int id=0;id<int(start.size());++id){
   int prev=start[id],k=0;
   for(auto task:crew.q[id]){
    if(task.mask){
     int relax=dist(prev,cap.stance)+dist(cap.stance,task.at)-dist(prev,task.at);
     if(relax>=0&&relax<=2){
      min_relax=std::min(min_relax,relax);
      auto key=std::tuple{finish[id]+1+relax,relax,id,k};
      if(key<best){best=key;crew_id=id;position=k;cost=relax;}
     }
     prev=task.at;
    }
    ++k;
   }
  }
  if(crew_id<0){++out.without;continue;}
  auto& q=crew.q[crew_id];q.insert(q.begin()+position,{cap.stance,1u<<cap.dir});
  finish[crew_id]+=1+cost;chosen[cap.gate]=1;++out.count;out.extra+=cost;out.min_extra+=min_relax;
 }
 for(int id=0;id<int(start.size());++id){
  auto from=fallback.q[id].begin()+stock.q[id].size();
  for(auto it=from;it!=fallback.q[id].end();++it){
   int g=E::nb(it->at)[std::countr_zero(it->mask)];assert(g>=0);
   if(!chosen[g])crew.q[id].push_back(*it),chosen[g]=1;
  }
  auto j=stock.q[id].begin();
  for(auto task:crew.q[id]){
   if(j!=stock.q[id].end()&&j->at==task.at&&j->mask==task.mask){++j;continue;}
   assert(std::popcount(task.mask)==1);
   int g=E::nb(task.at)[std::countr_zero(task.mask)];
   assert(g>=0&&!orig.planned(g)&&target.planned(g));
  }
  assert(j==stock.q[id].end());
 }
 assert(chosen.count()==8);
 crew.layout=layout(target);
 return out;
}
struct Result{
 int oldx=-1,allx=-1,staffed=-1,built=0,route_loss=0,stranded=0;
};
Result run(E::Crew crew,const T::Terrain& base,const T::Terrain& target,vector<int> people){
 M::State s;s.people=std::move(people);
 auto order=target.aisleColumns;
 std::stable_sort(order.begin(),order.end(),[](int a,int b){return std::abs(a-15)<std::abs(b-15);});
 array<int,30> expect{};
 for(int i=0;i<int(s.people.size());++i)++expect[order[i%order.size()]];
 Result r;
 for(int t=0;t<=145;++t){
  auto w=s.walls;auto& h=s.people;
  if(r.oldx<0&&(w&base.plannedWalls)==base.plannedWalls)r.oldx=t;
  if(r.allx<0&&(w&target.plannedWalls)==target.plannedWalls)r.allx=t;
  r.built=(w&target.plannedWalls).count();
  const auto ac=T::labels(w);int anchor=ac.labels[target.aisleColumns.front()];
  for(int p:h)if(ac.labels[p]!=anchor)++r.stranded;
  array<int,30> ct{};for(int p:h)++ct[p%30];
  if(r.allx>=0&&ct==expect){r.staffed=t;return r;}
  if(t==145)break;
  E::Board ew{};
  for(int q=0;q<V;++q)ew[q]=w[q];
  for(int i=0;i<int(h.size());++i)for(auto j:crew.q[i])if(j.mask){
   auto adj=E::nb(j.at);
   for(int d=0;d<4;++d)if((j.mask>>d&1)&&adj[d]>=0&&base.planned(adj[d])&&!w[adj[d]]){
    if(ac.labels[h[i]]!=ac.labels[j.at])++r.route_loss;
   }
  }
  auto step=crew.act_literal(ew,h,{});
  auto a=M::sanitize(s,vector<char>(step.action.begin(),step.action.end()));
  M::people_step(s,a);
  assert((s.walls&~target.plannedWalls).none());
 }
 return r;
}
int main(int argc,char** argv){
 bool advisory=argc==2&&std::string(argv[1])=="--advisory";
 std::mt19937 rng(0x3905d1b1);
 vector<int> p(V);std::iota(p.begin(),p.end(),0);
 const auto& base=T::terrain(3);auto target=augmented(3,base);
 for(int k:{0,1,2,4}){
  const auto& old=T::terrain(k);const auto x=augmented(k,old);
  assert(x.cells==old.cells&&x.doorCells==old.doorCells&&x.aisleColumns==old.aisleColumns&&x.plannedWalls==old.plannedWalls);
 }
 auto open=T::labels(target.plannedWalls);
 assert(open.count==1);
 auto before=T::classify(base,base.plannedWalls,{}, {0});
 auto after=T::classify(target,target.plannedWalls,{}, {0});
 assert(before.count()==75&&after.count()==75);
 vector<T::Board> m0(before.count()),m1(after.count());
 vector<int> room_to(after.count(),-1);
 for(int p=0;p<V;++p)if(!base.planned(p)&&!base.doorCells[p]){
  int i=before.ids[p],j=after.ids[p];
  assert((i<0)==(j<0));
  if(i>=0){m0[i][p]=true;m1[j][p]=true;assert(room_to[i]<0||room_to[i]==j);room_to[i]=j;}
 }
 int changed=0;
 for(int i=0;i<before.count();++i){
  int j=room_to[i];assert(j>=0&&m0[i]==m1[j]&&m0[i].any());
  auto l=before.rooms[i],r=after.rooms[j];
  if(l.left!=r.left||l.right!=r.right){
   ++changed;
   assert((r.left==l.left&&r.right<0&&target.planned(l.right))||
          (r.right==l.right&&r.left<0&&target.planned(l.left)));
   assert(r.size+1==l.size);
  }else assert(r.size==l.size);
 }
 assert(changed==8);
 std::cout<<"m,trial,stock_x,stock_staff,stock_built,stock_stranded,stock_route_loss,new_orig_x,new_x,new_staff,new_built,new_stranded,new_route_loss,splice_count,splice_added_walk,splice_min_walk,fallback_count,elapsed_ms\n";
 for(int m=5;m<=10;++m)for(int k=0;k<6;++k){
  const auto st=std::chrono::steady_clock::now();
  std::shuffle(p.begin(),p.end(),rng);vector<int> h(p.begin(),p.begin()+m);
  E::Crew a;assert(a.init(layout(base),h));auto b=a;
  auto extra=splice(b,base,target,h);
  if(advisory){std::cout<<m<<','<<k<<','<<extra.count<<','<<extra.extra<<','<<extra.min_extra<<','<<extra.without<<'\n'<<std::flush;continue;}
  auto stock=run(a,base,base,h),cand=run(b,base,target,h);
  auto dt=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-st).count();
  std::cout<<m<<','<<k<<','<<stock.allx<<','<<stock.staffed<<','<<stock.built<<','<<stock.stranded<<','<<stock.route_loss<<','
   <<cand.oldx<<','<<cand.allx<<','<<cand.staffed<<','<<cand.built<<','<<cand.stranded<<','<<cand.route_loss<<','<<extra.count<<','<<extra.extra<<','<<extra.min_extra<<','<<extra.without<<','<<dt<<'\n'<<std::flush;
 }
}
