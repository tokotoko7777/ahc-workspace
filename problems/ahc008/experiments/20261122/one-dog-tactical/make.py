from pathlib import Path
s=Path('tactical_original.cpp').read_text()
helper=Path('transit.hpp').read_text().removeprefix('#pragma once\n')
s=s.replace('using namespace std;',helper+'\nusing namespace std;',1)
s=s.replace(' Game g;g.init(argc,argv);',''' Game g;g.init(argc,argv);
 transit::DogEdgeTransit::PlanState trans;
 vector<int> pp(g.pets.size()), tt;
 for(const auto& pet:g.pets)tt.push_back(pet.t);
 const bool one_dog=count(tt.begin(),tt.end(),4)==1;
 if(one_dog){g.dogPhase=9; g.dogDecoy=-1; g.dogClosed=0;}
 ''')
s=s.replace('  g.act=g.terminal(g.tacticalGate(g.repair(g.plan())));','''  if(one_dog){
   for(int i=0;i<(int)pp.size();i++)pp[i]=g.pets[i].p;
   g.act=trans.plan(g.humans,pp,tt,g.wall);
  }
  if(!one_dog||g.act.empty())g.act=g.terminal(g.tacticalGate(g.repair(g.plan())));''')
s=s.replace('#ifdef GATE_DIAG\n cerr<<',''' if(getenv("TRANSIT_DIAG"))cerr<<"TRANSIT "<<one_dog<<" "<<trans.elapsed<<" "<<trans.wall_turns<<" "<<trans.guard_turns<<" "<<trans.builds<<" "<<trans.walks<<" "<<trans.waits<<" "<<trans.captured_dogs<<" "<<trans.captured_other<<"\\n";
#ifdef GATE_DIAG
 cerr<<''')
Path('tactical_edge.cpp').write_text(s)
