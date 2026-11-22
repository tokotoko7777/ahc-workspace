from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
def edit(x,y):
 global s
 assert s.count(x)==1,x
 s=s.replace(x,y,1)
edit(' array<int,300> integratedMode{},integratedReady{},integratedLocks{};',
''' array<int,300> integratedMode{},integratedReady{},integratedLocks{};
 vector<array<int,3>> doglegAudit;''')
edit('   maybeStartBridge(ready,usedPeople,build,moveTo,pairedTubes);',
'''   maybeStartBridge(ready,usedPeople,build,moveTo,pairedTubes);
   if(turn>=101&&turn<160){
    int mask=0;
    for(int i=0;i<m;i++)if(!usedPeople[i]&&act[i]=='.')mask|=1<<i;
    doglegAudit.push_back({turn,mask,total==n});
   }''')
needle=' if(const char* p=getenv("AHC008_GLOBAL_CLEAR_LOG")){'
edit(needle,''' if(const char* p=getenv("AHC008_DOGLEG_LOG")){
  ofstream f(p);
  for(auto r:g.doglegAudit)f<<r[0]<<" "<<r[1]<<" "<<r[2]<<'\\n';
 }
'''+needle)
Path(sys.argv[2]).write_text(s)
