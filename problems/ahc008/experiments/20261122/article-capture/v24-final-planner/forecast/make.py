from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
a=s.index('namespace lastcap {');b=s.index('namespace terry_maps::weights',a)
t=s[a:b]
def rep(p,q):
 global t
 assert t.count(p)==1,p
 t=t.replace(p,q)
rep(' Shape active;',' using ForecastGain=std::function<double(const Shape&,int,int)>;\n Shape active;')
rep('const std::vector<int>& humans,const std::vector<Pet>& pets,const Board& protect){',
    'const std::vector<int>& humans,const std::vector<Pet>& pets,const Board& protect,const ForecastGain& forecast){')
rep('''   z.value=(double(z.kept*(1<<z.trapped))-size)/(1.+.045*dur+.034*z.cut.size()+.012*near);''','''   auto gain=double(z.kept*(1<<z.trapped))-size;
   if(forecast)gain=.10*gain+.90*forecast(z,max(0,dur-1),size);
   z.value=gain/(1.+.045*dur+.034*z.cut.size()+.012*near);'''.replace('max(0,dur-1)','std::max(0,dur-1)'))
rep('const std::vector<Pet>& pets,const Board& protect={},const Board& retreat={}){',
    'const std::vector<Pet>& pets,const Board& protect={},const Board& retreat={},const ForecastGain& forecast={}){')
rep('active=search(turn,wall,root,size,humans,pets,protect);','active=search(turn,wall,root,size,humans,pets,protect,forecast);')
rep('active=search(turn,wall,root,size,humans,pets,retreat);','active=search(turn,wall,root,size,humans,pets,retreat,forecast);')
s=s[:a]+t+s[b:]
x='''   if(auto q=finalCapture.plan(turn,wall,humans,alive,roads,retreat)){'''
y='''   optional<terry_mc::Paths> known;
   lastcap::Planner::ForecastGain evaluate;
   if(m==5)evaluate=[&](const lastcap::Planner::Shape& shape,int eta,int area){
    if(!known){
     if(!postReady)observedMotionGraph();
     const auto prior=mcDogs();
     known=terry_mc::sample_paths(mcState(),terrySim,24,min(48,299-turn),&prior);
    }
    bitset<V> blocked;
    for(int p:shape.cut){blocked[p]=1;for(int u:nb[p])if(u>=0)blocked[u]=1;}
    double gain=0.;
    for(int trial=0;trial<known->trials;++trial){
     const auto& sample=known->at(trial,min(eta,known->horizon));
     bool legal=true;
     int hit=0;
     for(int j=0;j<int(pets.size());++j){
      int p=sample[j];
      if(blocked[p]){legal=false;break;}
      if(group[pets[j].p]==principal&&!shape.can[p])++hit;
     }
     if(legal&&hit)gain+=max(0,shape.kept*(1<<hit)-area);
    }
    return gain/max(1,known->trials);
   };
   if(auto q=finalCapture.plan(turn,wall,humans,alive,roads,retreat,evaluate)){'''
assert s.count(x)==1
s=s.replace(x,y)
Path(sys.argv[2]).write_text(s)
