from pathlib import Path
s=Path('cap4_original.cpp').read_text()
helper=Path('transit.hpp').read_text().removeprefix('#pragma once\n')
s=s.replace('using namespace std;',helper+'\nusing namespace std;',1)
s=s.replace(' Game g;g.init(argc,argv);',''' Game g;g.init(argc,argv);
 transit::DogEdgeTransit::PlanState trans;
 if(const char* cap=getenv("TRANSIT_LIMIT")) trans.max_turns=atoi(cap);
 vector<int> pp(g.pets.size()), tt;
 for(const auto& pet:g.pets)tt.push_back(pet.t);''')
s=s.replace('  g.act=g.terminal(g.tacticalGate(g.repair(g.plan())));','''  for(int i=0;i<(int)pp.size();i++)pp[i]=g.pets[i].p;
  g.act=trans.plan(g.humans,pp,tt,g.wall);
  if(g.act.empty())g.act=g.terminal(g.tacticalGate(g.repair(g.plan())));''')
s=s.replace('#ifdef GATE_DIAG\n cerr<<',''' if(getenv("TRANSIT_DIAG"))cerr<<"TRANSIT "<<trans.elapsed<<" "<<trans.wall_turns<<" "<<trans.guard_turns<<" "<<trans.all_staffed_turns<<" "<<trans.builds<<" "<<trans.walks<<" "<<trans.waits<<" "<<trans.captured_dogs<<" "<<trans.captured_other<<"\\n";
#ifdef GATE_DIAG
 cerr<<''')
Path('transit_cap4.cpp').write_text(s)
