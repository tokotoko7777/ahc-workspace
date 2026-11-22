from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
def edit(x,y):
 global s
 assert s.count(x)==1,x
 s=s.replace(x,y,1)
edit(' int phase2=0, target2=-1, bestPrev2=0;',
''' int phase2=0, target2=-1, bestPrev2=0;
 array<int,300> secondaryBankAudit{};''')
edit('  pocketPlan(p2,target2,phase2,bestPrev2,target);',
'''  pocketPlan(p2,target2,phase2,bestPrev2,target);
  if(bankMode&&bankRelease==160&&turn>=160&&turn<205)
   for(int i=0;i<hm;i++)if(p2[i])secondaryBankAudit[turn]|=1<<i;''')
needle=' if(const char* p=getenv("AHC008_GLOBAL_CLEAR_LOG")){'
edit(needle,''' if(const char* p=getenv("AHC008_SECONDARY_BANK_LOG")){
  ofstream f(p);
  for(int i=160;i<205;i++)if(g.secondaryBankAudit[i])
   f<<i<<" "<<g.secondaryBankAudit[i]<<'\\n';
 }
'''+needle)
Path(sys.argv[2]).write_text(s)
