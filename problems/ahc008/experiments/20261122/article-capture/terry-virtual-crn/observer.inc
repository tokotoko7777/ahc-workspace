#include <ctime>
#include <sstream>
#include <iomanip>
namespace crn_observer {
using namespace terry_exact_root;
enum Slot {Target,Directed,Random1,Random2,Random3,Fallback};
void keyed(State& s,int turn,keyed_crn::Words& w){
 HumanDistances ds{s,s.walls};
 for(int i=0;i<(int)s.pets.size();++i){
  auto& p=s.pets[i];
  auto begin=[&](int e){w.begin(turn,i,e);};
  if(p.type==4){
   int t=p.target,m=s.people.size();
   if(t<0||t>=m||ds.at(t)[p.pos]<=0||ds.at(t)[p.pos]>=INF){
    int list[10],n=0;
    for(int j=0;j<m;++j){const int d=ds.at(j)[p.pos];if(d>0&&d<INF)list[n++]=j;}
    if(!n){p.target=-1;begin(Fallback);random_step(p,s.walls,w,true);continue;}
    begin(Target);t=list[choose_small(w,n)];
   }
   p.target=t;begin(Directed);directed_step(p,s.walls,ds.at(t),w,true);
   if(p.pos==s.people[t])p.target=-1;
   begin(Random2);random_step(p,s.walls,w,true);
   if(p.pos==s.people[t])p.target=-1;
  }else if(p.type==5){
   Dist d;
   if(p.target>=0&&!s.walls[p.target]&&p.target!=p.pos){
    bfs(s.walls,p.target,d);if(d[p.pos]>=INF)p.target=-1;
   }else p.target=-1;
   if(p.target<0){
    bfs(s.walls,p.pos,d);int list[V],n=0;
    for(int j=0;j<V;++j)if(d[j]>0&&d[j]<INF)list[n++]=j;
    if(!n)continue;
    begin(Target);p.target=list[choose_small(w,n)];bfs(s.walls,p.target,d);
   }
   begin(Directed);directed_step(p,s.walls,d,w,true);
   if(p.pos==p.target)p.target=-1;
   begin(Random2);random_step(p,s.walls,w,true);
   if(p.pos==p.target)p.target=-1;
  }else for(int j=0;j<p.type;++j){begin(Random1+j);random_step(p,s.walls,w,true);}
 }
}
bool same(const State& a,const State& b){
 if(a.people!=b.people||a.walls!=b.walls||a.pets.size()!=b.pets.size())return false;
 for(int i=0;i<(int)a.pets.size();++i)if(a.pets[i].pos!=b.pets[i].pos||a.pets[i].target!=b.pets[i].target)return false;
 return true;
}
bool full(const State& s){
 auto c=terrain_lib::labels(s.walls);std::vector<bool> human(c.count);
 for(int p:s.people)human[c.labels[p]]=true;
 for(auto p:s.pets)if(human[c.labels[p.pos]])return false;
 return true;
}
void self_test(){
 State s;
 s.people={303,604};s.pets={{310,1,-1},{310,2,-1},{310,3,-1},{310,4,-1},{310,5,-1}};
 State a=s,b=s;
 keyed_crn::Words ra{773},rb{773};
 for(int t=275;t<300;++t){keyed(a,t,ra);keyed(b,t,rb);assert(same(a,b));}
 // Detect gross probability implementation error with fixed synthetic cross-sections.
 State c;c.pets={{310,1,-1}};
 std::array<int,4> out{};
 for(unsigned seed=0;seed<32768;++seed){
  State x=c;keyed_crn::Words w{seed};keyed(x,275,w);
  for(int j=0;j<4;++j)if(x.pets[0].pos==nb[310][j])++out[j];
 }
 for(int k:out)assert(std::abs(k-8192)<400);
}
int run(int argc,char** argv){
 if(argc!=7)return 2;
 std::ifstream f(argv[1]),trace(argv[2]); int n,m;
 f>>n;State live;
 std::array<int,5> count{};
 for(int i=0;i<n;++i){int r,c,k;f>>r>>c>>k;live.pets.push_back({(r-1)*30+c-1,k,-1});++count[k-1];}
 f>>m;
 for(int i=0;i<m;++i){int r,c;f>>r>>c;live.people.push_back((r-1)*30+c-1);}
 assert(f&&trace); // hidden generator suffix unread
 const int id=terry_exact_selector::choose(m,count);
 const auto& land=terrain_lib::terrain(id);
 Normal initial(land,live);
 for(int t=0;t<275;++t){
  std::string a,line;assert(std::getline(trace,a)&&std::getline(trace,line));
  assert(initial.step(live,t)==a);
  people_step(live,a);
  std::istringstream si(line);
  for(auto& p:live.pets){
   std::string s;si>>s;assert(si);
   for(char c:s){int d=move_dir(c);assert(d>=0);p.pos=nb[p.pos][d];assert(p.pos>=0&&!live.walls[p.pos]);}
  }
 }
 const std::string mode=argv[3];
 if(mode=="verify"){
  auto normal=initial;
  const int cap=std::stoi(argv[4]);
  const auto d=decide(live,normal,cap,Clock::time_point::max());
  assert(d.pairs==cap&&d.normal==std::stoll(argv[5])&&d.chase_sum==std::stoll(argv[6]));
  std::cout<<"verified\n";return 0;
 }
 assert(mode.size()==3);
 // Alternate arm order externally. Run model work only after all cases verify.
 std::cout<<std::setprecision(16);
 for(char method:mode){
  assert(method=='S'||method=='K'||method=='I');
  const std::clock_t start=std::clock();
  auto normal=initial;
  const auto first=chase(live,land,275);
  std::array<std::array<int,4>,16> result{};
  for(int i=0;i<16;++i){
   const uint64_t seed=998244353ULL*(i+1);
   State a=live,b=live;
   auto save=normal.rng;normal.rng=Pcg(42);
   ChaCha p(seed),q(method=='I'?keyed_crn::mix(seed^0xd35b744a6a1ffde1ULL):seed);
   keyed_crn::Words kp{seed},kq{seed};
   for(int t=275;t<300;++t){
    people_step(a,normal.step(a,t));
    if(method=='K')keyed(a,t,kp);else actual_motion(a,p);
   }
   normal.rng=save;
   for(int t=275;t<300;++t){
    if(t==275)people_step(b,first);
    else{
     bool empty=false;auto cmd=chase(b,land,275,&empty);
     if(empty)break;
     people_step(b,cmd);
    }
    if(method=='K')keyed(b,t,kq);else actual_motion(b,q);
   }
   result[i]={score(a),score(b),full(a),full(b)};
  }
  auto elapsed=double(std::clock()-start)/CLOCKS_PER_SEC;
  std::cout<<method<<','<<elapsed;
  for(auto v:result)for(int x:v)std::cout<<','<<x;
  std::cout<<'\n';
 }
 return 0;
}
}
int main(int argc,char** argv){
 if(argc==2&&std::string(argv[1])=="unit"){crn_observer::self_test();return 0;}
 return crn_observer::run(argc,argv);
}
