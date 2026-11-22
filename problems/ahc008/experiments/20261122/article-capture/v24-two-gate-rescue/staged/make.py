from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
k=' string integratedBankPlan(){'
assert s.count(k)==1
s=s.replace(k,Path(__file__).with_name('pair.inc').read_text()+'\n'+k)
k='  if(turn>=bankRelease){integratedMode[turn]=4;dogPhase=9;return plan();}'
assert s.count(k)==1
s=s.replace(k,'''  if(turn>=bankRelease){
   integratedMode[turn]=4;dogPhase=9;
   auto base=plan();
   if(auto cut=pairGate(rem,base)){integratedMode[turn]=(pairStageAt==turn?9:8);return *cut;}
   return base;
  }''')
Path(sys.argv[2]).write_text(s)
