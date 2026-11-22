from pathlib import Path
import sys
base,out=map(Path,sys.argv[1:])
root=Path(__file__).resolve().parent
src=base.read_text()
cut0='  // Untasked workers build local brace frontiers without disrupting a paired hold.\n'
cut1='  if(scheduled&&!partial){\n   maybeStartBridge'
assert src.count(cut0)==src.count(cut1)==1
a=src.index(cut0); b=src.index(cut1,a)
assert '20+2*progress-6*d' in src[a:b]
src=src[:a]+(root/'contractor.inc').read_text()+src[b:]
at=' string integratedBankPlan(){'
assert src.count(at)==1
src=src.replace(at,''' // Per-incident-pen inverse remaining work; one wall to ready receives
 // exact full credit. A shared shoulder supports both bordering pens.
 static int bankBraceCredit(int n){
  return n>0?12/n:0;
 }
 static int bankPartnerTie(int local,int other){
  return local<=2&&other<=4?5-other:0;
 }

'''+at)
out.write_text(src)
