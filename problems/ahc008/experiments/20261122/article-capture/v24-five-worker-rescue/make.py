from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
insert=' string integratedBankPlan(){'
assert s.count(insert)==1
s=s.replace(insert,Path(__file__).with_name('spine.inc').read_text()+'\n'+insert)
a='  if(turn>=bankRelease){integratedMode[turn]=4;dogPhase=9;return plan();}'
assert s.count(a)==1
s=s.replace(a,'''  if(turn>=bankRelease){
#ifdef AHC008_SPINE_MERGE
   integratedMode[turn]=4;dogPhase=9;
   auto base=plan();
   if(auto cut=spineCut(rem,&base)){integratedMode[turn]=7;return *cut;}
   return base;
#else
   if(auto cut=spineCut(rem)){integratedMode[turn]=7;return *cut;}
#endif
   integratedMode[turn]=4;dogPhase=9;return plan();
  }''')
a=' if(const char* p=getenv("AHC008_TERRY_LOG")){'
assert s.count(a)==1
s=s.replace(a,''' if(const char* p=getenv("AHC008_SPINE_LOG")){
  ofstream f(p);f<<g.spineSteps<<" "<<g.spineClosed<<'\\n';
 }
'''+a)
Path(sys.argv[2]).write_text(s)
