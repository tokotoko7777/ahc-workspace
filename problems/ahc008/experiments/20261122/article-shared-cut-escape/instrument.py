import sys
from pathlib import Path
s=Path(sys.argv[1]).read_text()
def edit(old,new):
 global s
 assert s.count(old)==1,old
 s=s.replace(old,new,1)
edit(' Shape active;','''
 Shape active;
 struct RetryAudit {int kind,turn,key,tier;std::vector<int> cut;};
 std::vector<RetryAudit> retryAudit;
 int auditTier=0;''')
edit('   z.cost=dur;','''
   retryAudit.push_back({1,turn,0,auditTier,z.cut});
   z.cost=dur;''')
edit('  for(int h:humans)if(rootB.d[h]<0){if(began>=0)reset(turn);return std::nullopt;}',
'''  for(int h:humans)if(rootB.d[h]<0){
   if(began>=0){retryAudit.push_back({4,turn,began,protectionTier,active.cut});reset(turn);}
   return std::nullopt;
  }''')
edit('   if(turn<deadline)++escapes;',
'''   if(turn<deadline)++escapes;
   if(turn<deadline&&total<cfg.maxTotal)
    retryAudit.push_back({4,turn,began,protectionTier,active.cut});''')
edit('   active=search(turn,wall,root,size,humans,pets,protect);',
'''   auditTier=1;
   retryAudit.push_back({0,turn,0,auditTier,{}});
   active=search(turn,wall,root,size,humans,pets,protect);''')
edit('    active=search(turn,wall,root,size,humans,pets,retreat);',
'''    auditTier=2;
    retryAudit.push_back({0,turn,0,auditTier,{}});
    active=search(turn,wall,root,size,humans,pets,retreat);''')
edit('   began=turn;deadline=std::min({300,turn+cfg.maxAttempt,turn+cfg.maxTotal-total});++attempts;',
'''   began=turn;deadline=std::min({300,turn+cfg.maxAttempt,turn+cfg.maxTotal-total});++attempts;
   retryAudit.push_back({2,turn,began,protectionTier,active.cut});''')
edit('if(finalSince>=0&&turn-finalSince>=cfg.finalWait){reset(turn);return std::nullopt;}',
'''if(finalSince>=0&&turn-finalSince>=cfg.finalWait){
      retryAudit.push_back({5,turn,began,protectionTier,active.cut});reset(turn);return std::nullopt;
     }''')
edit("  if(out.find_first_not_of('.')==std::string::npos){reset(turn);return std::nullopt;}",
'''  if(out.find_first_not_of('.')==std::string::npos){
   retryAudit.push_back({6,turn,began,protectionTier,active.cut});reset(turn);return std::nullopt;
  }''')
needle=' if(const char* p=getenv("AHC008_GLOBAL_CLEAR_LOG")){'
edit(needle,''' if(const char* p=getenv("AHC008_RETRY_AUDIT")){
  ofstream f(p);
  for(const auto& x:g.finalCapture.retryAudit){
   f<<x.kind<<" "<<x.turn<<" "<<x.key<<" "<<x.tier;
   for(int q:x.cut)f<<" "<<q;
   f<<'\\n';
  }
 }
'''+needle)
Path(sys.argv[2]).write_text(s)
