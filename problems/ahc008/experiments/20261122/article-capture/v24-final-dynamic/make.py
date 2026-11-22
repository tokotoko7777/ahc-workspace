from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
a=s.index('namespace lastcap {');b=s.index('namespace terry_maps::weights',a)
t=s[a:b]
def rep(p,q):
 global t
 assert t.count(p)==1,p
 t=t.replace(p,q)
rep(' Shape active;',' using RollChoices=std::function<int(const std::vector<Shape>&)>;\n Shape active;')
rep('const std::vector<int>& humans,const std::vector<Pet>& pets,const Board& protect){',
    'const std::vector<int>& humans,const std::vector<Pet>& pets,const Board& protect,const RollChoices& pick){')
rep('  Shape best;\n  const int n=ap.size()', '  Shape best;\n  std::vector<Shape> choices;\n  const int n=ap.size()')
rep('''   if(z.value>best.value)best=std::move(z);
  }
  return best;''','''   if(pick){
    if(z.value<0)continue;
    if(std::any_of(choices.begin(),choices.end(),[&](const Shape& p){return p.cut==z.cut;}))continue;
    auto it=std::find_if(choices.begin(),choices.end(),[&](const Shape& p){return p.value<z.value;});
    if(it!=choices.end()||choices.size()<3)choices.insert(it,std::move(z));
    if(choices.size()>3)choices.pop_back();
   }else if(z.value>best.value)best=std::move(z);
  }
  if(!choices.empty()){
   int selected=choices.size()>1?std::clamp(pick(choices),0,int(choices.size()-1)):0;
   return std::move(choices[selected]);
  }
  return best;''')
rep('const std::vector<Pet>& pets,const Board& protect={},const Board& retreat={}){',
    'const std::vector<Pet>& pets,const Board& protect={},const Board& retreat={},const RollChoices& pick={}){')
rep('active=search(turn,wall,root,size,humans,pets,protect);','active=search(turn,wall,root,size,humans,pets,protect,pick);')
rep('active=search(turn,wall,root,size,humans,pets,retreat);','active=search(turn,wall,root,size,humans,pets,retreat,pick);')
s=s[:a]+t+s[b:]
x=' lastcap::Planner finalCapture;'
assert s.count(x)==1
s=s.replace(x,x+'\n clock_t dynamicFinalSpent=0;\n int dynamicFinalPairs=0,dynamicFinalChanged=0;')
x='''   if(auto q=finalCapture.plan(turn,wall,humans,alive,roads,retreat)){'''
y='''   lastcap::Planner::RollChoices pick;
   constexpr clock_t spend=clock_t(.060*CLOCKS_PER_SEC);
   if(m==5&&dynamicFinalSpent<spend)pick=[&](const vector<lastcap::Planner::Shape>& shapes){
    const clock_t now=clock();
    const clock_t deadline=min({started+clock_t(1.55*CLOCKS_PER_SEC),
       now+min(spend-dynamicFinalSpent,clock_t(.014*CLOCKS_PER_SEC))});
    if(deadline<=now)return 0;
    if(!postReady)observedMotionGraph();
    const auto prior=mcDogs();
    auto initial=mcState();
    auto buildPolicy=[&](const lastcap::Planner::Shape& z){
     lastcap::Planner state=finalCapture;
     state.active=z;state.began=turn;state.deadline=min(300,turn+state.cfg.maxAttempt);
     return [state,&sim=terrySim](const terry_mc::View& v)mutable->terry_mc::Moves{
      // Follow only this preselected enclosure, and stop after its
      // cancellation. This compares actual partial construction trajectories
      // without searching or changing tactics again inside a rollout.
      if(state.began<0)return {};
      vector<lastcap::Pet> p;
      p.reserve(v.n);
      for(int j=0;j<v.n;++j)p.push_back({v.pet[j],v.type[j]});
      if(auto act=state.plan(v.turn,v.wall,v.humans,p))
       return terry_proxy::decode(v,sim,*act);
      return {};
     };
    };
    int steps=min(16,300-turn);
    int winner=0;
    double margin=0.;
    for(int i=1;i<int(shapes.size());++i){
     auto r=terry_mc::paired(initial,terrySim,buildPolicy(shapes[0]),buildPolicy(shapes[i]),
                  terry_mc::Options{6,steps,deadline},&prior);
     dynamicFinalPairs+=r.pairs;
     if(r.pairs>=4&&r.delta>r.stderr&&r.delta-r.stderr>margin)winner=i,margin=r.delta-r.stderr;
    }
    dynamicFinalSpent+=clock()-now;dynamicFinalChanged+=(winner!=0);
    return winner;
   };
   if(auto q=finalCapture.plan(turn,wall,humans,alive,roads,retreat,pick)){'''
assert s.count(x)==1
s=s.replace(x,y)
x=' if(const char* p=getenv("AHC008_TERRY_LOG")){'
assert s.count(x)==1
s=s.replace(x,''' if(const char* p=getenv("AHC008_DYNAMIC_FINAL_LOG")){
  ofstream f(p);f<<g.dynamicFinalPairs<<" "<<g.dynamicFinalChanged<<" "<<double(g.dynamicFinalSpent)/CLOCKS_PER_SEC<<'\\n';
 }
'''+x)
Path(sys.argv[2]).write_text(s)
