// Included in the standalone source after lastcap. No repo-local dependency.
namespace cordon {
using namespace lastcap;
struct Front {
 Planner searcher;
 std::vector<int> cut, gates, crew;
 Board exterior{};
 int start=-1, retry=0, outsideTicks=0, initialSize=0, kept=0;
 int attempts=0, shellWalls=0, closes=0, aborted=0, heldOutside=0, groundGain=0;
 Front(){
  searcher.cfg.maxAttempt=28;
  searcher.cfg.maxTotal=28;
  searcher.cfg.maxWalls=16;
  searcher.cfg.minArea=300;
 }
 void clear(int turn,bool success){
  if(start>=0&&!success)++aborted;
  start=-1;retry=turn+7;cut.clear();gates.clear();crew.clear();
 }
 bool init(int turn,const Board& wall,const Board& root,int size,
           const std::vector<int>& h,const std::vector<Pet>& pets){
  // First cut is a persistent outer shell, not a moving target rectangle.
  // A larger protected core than the old final-cut planner tolerates has
  // time to absorb a short excursion while the rest of the crew builds.
  Board protect{};protect[h[0]]=1;
  std::vector<Pet> passive;
  for(auto a:pets)if(a.t<=3)passive.push_back(a);
  auto shape=searcher.search(turn,wall,root,size,h,passive,protect,2,4);
  if(shape.value<0||shape.kept<300||size-shape.kept>160)return false;
  cut=std::move(shape.cut);exterior=shape.can;kept=shape.kept;initialSize=size;
  int n=cut.size(),m=h.size();
  std::array<BFS,10> nav;
  for(int a=0;a<m;a++)nav[a].run(h[a],exterior);
  std::vector<std::array<int,10>> ds(n);
  std::vector<int> inward(n);
  for(int j=0;j<n;j++){
   ds[j].fill(INF);
   for(int p:neighbors(cut[j]))if(p>=0){
    inward[j]+=!wall[p]&&!exterior[p]&&std::find(cut.begin(),cut.end(),p)==cut.end();
    if(exterior[p])for(int a=0;a<m;a++)if(nav[a].d[p]>=0)ds[j][a]=std::min(ds[j][a],nav[a].d[p]);
   }
  }
  // Leave 1–2 low-conductance, staffable mouths, rather than whichever
  // arbitrary walls happen to remain last. Keep rest of crew on the shell.
  int best=INF;
  for(int j=0;j<n;j++)for(int a=0;a<m;a++)if(inward[j]>0&&ds[j][a]<=9)
   for(int k=-1;k<n;k++)if((n==1)==(k<0)&&k!=j)
    for(int b=0;b<(k<0?1:m);b++)if(k<0||(inward[k]>0&&b!=a&&ds[k][b]<=9)){
     int d=k<0?0:ds[k][b];
     int v=3*std::max(ds[j][a],d)+ds[j][a]+d+5*(inward[j]+(k<0?0:inward[k]));
     if(v<best){
      best=v;gates={cut[j]};crew={a};
      if(k>=0){gates.push_back(cut[k]);crew.push_back(b);}
     }
    }
  if(best==INF)return false;
  start=turn;outsideTicks=0;++attempts;
  return true;
 }
 std::optional<std::string> act(int turn,const Board& wall,
           const std::vector<int>& h,const std::vector<Pet>& pets) {
  if(turn<168||turn>297||h.size()<4)return std::nullopt;
  Board open=Planner::walkable(wall);
  BFS r;int size=r.run(h[0],open);
  Board root{};
  int count=0,passive=0;
  for(auto a:pets)if(r.d[a.p]>=0){count++;passive+=a.t<=3;}
  for(int p=0;p<V;p++)root[p]=r.d[p]>=0;
  bool together=std::all_of(h.begin(),h.end(),[&](int p){return bool(root[p]);});
  if(!together||size<320||!count||count>3){
   if(start>=0)clear(turn,false);
   return std::nullopt;
  }
  if(start<0){
   if(turn<retry||turn>275||!passive)return std::nullopt;
   if(!init(turn,wall,root,size,h,pets)){retry=turn+7;return std::nullopt;}
  }
  int inside=0;
  for(auto a:pets)if(root[a.p]&&!exterior[a.p]
     &&std::find(cut.begin(),cut.end(),a.p)==cut.end())inside++;
  if(inside)outsideTicks=0;
  else {outsideTicks++;heldOutside++;}
  if(turn-start>=24||outsideTicks>7||
      !std::all_of(h.begin(),h.end(),[&](int p){return bool(exterior[p]);})){
   clear(turn,false);return std::nullopt;
  }
  const int m=h.size();
  std::array<BFS,10> nav;
  for(int a=0;a<m;a++)nav[a].run(h[a],exterior);
  const auto blocked=Planner::occupied(h,pets);
  auto stance=[&](int i,int q){
   int p=-1,d=INF;
   for(int x:neighbors(q))if(x>=0&&nav[i].d[x]>=0&&nav[i].d[x]<d)p=x,d=nav[i].d[x];
   return std::pair(d,p);
  };
  std::string out(m,'.');
  std::array<bool,10> tasked{};
  std::vector<int> pending;
  for(int q:cut)if(!wall[q]&&std::find(gates.begin(),gates.end(),q)==gates.end())pending.push_back(q);
  bool canClose=inside>0&&pending.empty();
  for(int g=0;g<int(gates.size());g++){
   int a=crew[g],q=gates[g];auto [d,p]=stance(a,q);tasked[a]=true;
   if(!wall[q]){
    if(d!=0||blocked[q])canClose=false;
    if(d>0&&d<INF)out[a]=MOVE[direction(h[a],nav[a].first[p])];
   }
  }
  if(canClose){
   for(int g=0;g<int(gates.size());g++)if(!wall[gates[g]])
    out[crew[g]]=BUILD[direction(h[crew[g]],gates[g])];
   if(out.find_first_not_of('.')!=std::string::npos){
    ++closes;groundGain+=initialSize-kept;
    clear(turn,true);return out;
   }
  }
  using Job=std::tuple<int,int,int,int>;
  std::vector<Job> jobs;
  for(int j=0;j<int(pending.size());j++){
   int q=pending[j],traffic=0;
   for(int x:neighbors(q))if(x>=0&&!wall[x]&&!exterior[x])traffic++;
   for(int a=0;a<m;a++)if(!tasked[a]){
    auto [d,p]=stance(a,q);
    if(d<INF&&(d||!blocked[q]))
     jobs.emplace_back(8*d+4*blocked[q]-5*traffic,a,j,p);
   }
  }
  std::sort(jobs.begin(),jobs.end());
  std::array<bool,16> picked{};
  for(auto [score,a,j,p]:jobs)if(!tasked[a]&&!picked[j]){
   tasked[a]=picked[j]=true;int q=pending[j];
   if(h[a]==p){out[a]=BUILD[direction(h[a],q)];++shellWalls;}
   else out[a]=MOVE[direction(h[a],nav[a].first[p])];
  }
  // Keep the selected mouths staffed whether or not they are occupied this
  // frame, instead of moving along with the target animal. All stances,
  // planned walks and future walls are disjoint by the frozen certificate.
  return out;
 }
};
}
