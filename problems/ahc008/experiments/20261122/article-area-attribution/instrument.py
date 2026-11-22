from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
needle=' bool latePlan(){'
s=s.replace(needle,''' struct LateAreaTick{
  int episode=-1,keepSize=-1;double keepValue=-1;
  std::vector<int> goals,rawWalls;
 };
 std::array<LateAreaTick,300> areaDiag;
'''+needle,1)
anchor='''  int nb=0,remain=toBuild.size();
  array<unsigned char,V> assigned{};'''
assert s.count(anchor)==1
needle='''   assigned[p]=1;
  }
  return true;
 }
 void form(){'''
assert s.count(needle)==1
s=s.replace(needle,'''   assigned[p]=1;
  }
  areaDiag[turn].episode=lineStart;
  areaDiag[turn].keepSize=lcount[keep];
  areaDiag[turn].keepValue=gain;
  areaDiag[turn].goals=toBuild;
  return true;
 }
 void form(){''',1)
needle='    proposal=g.repair(std::move(proposal));'
assert s.count(needle)==1
s=s.replace(needle,'''    for(int i=0;i<(int)proposal.size();i++)if(islower(proposal[i]))
     g.areaDiag[g.turn].rawWalls.push_back(nb[g.humans[i]][find(BUILD,BUILD+4,proposal[i])-BUILD]);
'''+needle,1)
needle=' if(const char* p=getenv("AHC008_GLOBAL_CLEAR_LOG")){'
assert s.count(needle)==1
s=s.replace(needle,''' if(const char* p=getenv("AHC008_LATE_AREA_LOG")){
  ofstream f(p);
  for(int t=0;t<300;t++){
   const auto& x=g.areaDiag[t];
   if(x.episode<0&&x.rawWalls.empty())continue;
   f<<t<<" "<<g.integratedMode[t]<<" "<<x.episode<<" "<<x.keepSize<<" "
    <<setprecision(17)<<x.keepValue<<" "<<x.rawWalls.size()<<" "<<x.goals.size();
   for(int p:x.rawWalls)f<<" "<<p;
   for(int p:x.goals)f<<" "<<p;
   f<<'\\n';
  }
 }
'''+needle,1)
Path(sys.argv[2]).write_text(s)
