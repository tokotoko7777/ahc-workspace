from pathlib import Path
s=Path('transit_overlap.cpp').read_text()
s=s.replace('using namespace std;',Path('dog-target-filter.hpp').read_text().removeprefix('#pragma once\n')+'\nusing namespace std;',1)
s=s.replace(' string act;',''' string act;
 vector<AHC008DogTargetFilter> dogPosterior;
 bool dogPredict=true;
 int dogHiddenBad=0;''',1)
s=s.replace('  form();\n }','''  for(auto p:pets)dogPosterior.emplace_back(humans.size());
  form();
 }''',1)
a=s.index(' bool advance(){')
b=s.index('\n};\n}',a)
old=s[a:b]
s=s[:a]+old.replace('  string m;\n  for(auto &p:pets){','''  string m;
  vector<transit::DogEdgeTransit::Dist> d;
  if(dogPredict&&!dogPosterior.empty())for(int h:humans)d.push_back(transit::DogEdgeTransit::bfs(wall,h));
  for(int i=0;i<(int)pets.size();i++){
   auto &p=pets[i];
   int start=p.p, finish[3],n=0;''').replace("   for(char x:m)if(x!='.')p.p=nb[p.p][find(MOVE,MOVE+4,x)-MOVE];","""   for(char x:m)if(x!='.')finish[n++]=p.p=nb[p.p][find(MOVE,MOVE+4,x)-MOVE];
   if(dogPredict&&p.t==4&&n>0) {
    int next[4],k=0;
    for(int q:nb[start])if(q>=0&&!wall[q])next[k++]=q;
    auto dd=[&](int j,int p)->int{
     auto v=d[j][p];return v==transit::DogEdgeTransit::INF?-1:v;
    };
    if(!dogPosterior[i].update(start,span(next,k),span(finish,n),dd)){
     dogPosterior[i]=AHC008DogTargetFilter(humans.size());++dogHiddenBad;
    }
   }""")+s[b:]
s=s.replace('  if(g.act.empty())g.act=g.terminal(g.tacticalGate(g.repair(g.plan())));','''  if(g.act.empty()){
   g.dogPredict=false;
   g.act=g.terminal(g.tacticalGate(g.repair(g.plan())));
  }''')
old='''   int keep=2;
   int gnear=trans.guard;
   if(g.humans[gnear]==trans.z.a)for(int j=0;j<(int)g.humans.size();j++)
    if(j!=gnear&&!released[j]&&g.humans[j]==trans.z.b){
     if(keep>0)--keep;
     else released[j]=1;
    }'''
assert old in s
s=s.replace(old,'''   int gnear=trans.guard, nfar=0;
   for(int j=0;j<(int)g.humans.size();j++)nfar+=j!=gnear&&!released[j]&&g.humans[j]==trans.z.b;
   if(nfar>2&&g.humans[gnear]==trans.z.a){
    auto da=transit::DogEdgeTransit::bfs(g.wall,trans.z.a);
    auto db=transit::DogEdgeTransit::bfs(g.wall,trans.z.b);
    auto de=transit::DogEdgeTransit::bfs(g.wall,trans.z.lane[0]);
    array<double,10> risk{};
    bool near=false;
    for(int i=0;i<(int)g.pets.size();i++)if(g.pets[i].t==4){
     int p=g.pets[i].p;
     if(da[p]==transit::DogEdgeTransit::INF)continue;
     double far_mass=0, reset=g.dogPosterior[i].reset_probability();
     for(int j=0;j<(int)g.humans.size();j++)if(j!=gnear&&!released[j]&&g.humans[j]==trans.z.b)
      far_mass+=g.dogPosterior[i].probability(j);
     bool approach=db[p]>db[trans.z.a]&&db[p]==int(de[p])+db[trans.z.lane[0]];
     if(approach&&da[p]<=7&&far_mass>=.70)near=true;
     // Safeguard all still-separated probable targets, weighting imminent
     // approaches more than remote animals that can be retargeted many times.
     const double weight=approach&&da[p]<=7?1.:.28;
     for(int j=0;j<(int)g.humans.size();j++)
      risk[j]+=weight*(g.dogPosterior[i].probability(j)+reset/g.humans.size());
    }
    if(near)for(int j=0;j<(int)g.humans.size()&&nfar>2;j++)
     if(j!=gnear&&!released[j]&&g.humans[j]==trans.z.b&&risk[j]<.09){
      released[j]=1;
      --nfar;
     }
   }''')
s=s.replace('<<scaffold_moves<<"\\n";','<<scaffold_moves<<"\\n";')
Path('transit_informed.cpp').write_text(s)
