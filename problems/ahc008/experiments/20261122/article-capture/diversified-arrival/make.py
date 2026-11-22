"""Apply one self-contained, no-argument weighted coverage variant to a frozen source."""
from pathlib import Path
import sys
src=Path(sys.argv[1]).read_text()
def rep(old,new):
    global src
    assert src.count(old)==1,old[:120]
    src=src.replace(old,new)
rep(' struct BankJob{int tube,a,b,eta;double value;};',''' using PetCredit=array<float,20>;
 struct BankJob{int tube,a,b,eta;double value;};
 long long coveragePlans=0,coverageChanges=0;
 double coverageOldDuplicate=0.,coverageNewDuplicate=0.;''')
rep(' static int comp(int a,',''' // Existing worker-mask DP provides the best additive plan; a bounded
 // single-pair replacement then optimizes the genuinely diversified score.
 // Per-pet max credit is the primary first assignment; a secondary pair's
 // value is discounted rather than forbidden, to retain useful insurance.
 vector<BankJob> diversifyBankJobs(const vector<vector<BankJob>>& jobs,vector<BankJob> plan,
      const vector<array<PetCredit,Horizon+1>>& fraction){
  if(plan.size()<2)return plan;
  const int np=pets.size();
  auto duplicate=[&](const vector<BankJob>& q){
   PetCredit best{};
   double sum=0.;
   for(const auto& x:q)for(int p=0;p<np;p++){
    const float v=float(x.value)*fraction[x.tube][x.eta][p];
    sum+=v;best[p]=max(best[p],v);
   }
   return sum-accumulate(best.begin(),best.end(),0.);
  };
  coveragePlans++;
  coverageOldDuplicate+=duplicate(plan);
  bool changed=false;
  for(int round=0;round<2;round++){
   bool progress=false;
   for(int i=0;i<int(plan.size());i++){
    int used=0;bitset<V> usedTube;PetCredit covered{};
    for(int j=0;j<int(plan.size());j++)if(j!=i){
     auto& x=plan[j];used|=(1<<x.a)|(1<<x.b);usedTube[x.tube]=1;
     const auto& v=fraction[x.tube][x.eta];
     for(int p=0;p<np;p++)covered[p]=max(covered[p],float(x.value)*v[p]);
    }
    const auto marginal=[&](const BankJob& x){
     double independent=0.;
     const auto& v=fraction[x.tube][x.eta];
     for(int p=0;p<np;p++)independent+=max(0.f,float(x.value)*v[p]-covered[p]);
     return .4*x.value+.6*independent;
    };
    auto best=plan[i];double val=marginal(best);
    for(int t=0;t<int(jobs.size());t++)if(!usedTube[t])for(const auto& x:jobs[t]){
     if(used&((1<<x.a)|(1<<x.b)))continue;
     // The candidate's score alone upper-bounds its marginal.
     if(x.value<=val+1e-8)continue;
     double v=marginal(x);
     if(v>val+1e-8){val=v;best=x;}
    }
    if(best.tube!=plan[i].tube||best.a!=plan[i].a||best.b!=plan[i].b){
     plan[i]=best;progress=changed=true;
    }
   }
   if(!progress)break;
  }
  coverageChanges+=changed;
  coverageNewDuplicate+=duplicate(plan);
  return plan;
 }
 static int comp(int a,''')
rep('   if(maxEta&&!overBudget()){',
'''   vector<array<PetCredit,Horizon+1>> discountBase;
   if(maxEta&&!overBudget()){
    if(!partial)discountBase.resize(n);''')
rep('    for(int pi=0;pi<int(pets.size());pi++)if(service[group[pets[pi].p]]){',
'''
    for(int pi=0;pi<int(pets.size());pi++)if(service[group[pets[pi].p]]){''')
rep('       mass[k][h]=mass[k][h]*(1.-bad)+noBad[k][h]*best;',
'''       // Condition on this pet not blocking its own target gates;
       // the product over all pets is applied when creating the jobs.
       if(!partial&&best>0.&&1.-bad>1e-12)discountBase[k][h][pi]=float(best/(1.-bad));
       mass[k][h]=mass[k][h]*(1.-bad)+noBad[k][h]*best;''')
rep('    for(int k=0;k<n;k++){\n     array<BankJob,1024> best{};', '''    if(!partial)for(int k=0;k<n;k++)if(!pairInfo[k].empty())for(int h=1;h<=maxEta;h++){
     auto& v=discountBase[k][h];const float sum=accumulate(v.begin(),v.end(),0.f);
     if(sum>0.f)for(float& q:v)q/=sum;  // conditional first-capture credit fraction
    }
    for(int k=0;k<n;k++){
     array<BankJob,1024> best{};''')
rep('    for(auto j:packBankJobs(jobs,partial?1:m/2)){',
'''    auto selections=packBankJobs(jobs,partial?1:m/2);
    for(auto j:diversifyBankJobs(jobs,std::move(selections),discountBase)){''')
# Alternate source variants share this independent opt-in telemetry function.
mark='AHC008_ARTICLE_LOG'
pos=src.index(mark)
if 'AHC008_COVERAGE_LOG' not in src:
    insert=src.rfind('\n',0,pos)
    src=src[:insert]+'''
 if(const char* p=getenv("AHC008_COVERAGE_LOG")){
  ofstream out(p);
  out<<g.coveragePlans<<" "<<g.coverageChanges<<" "<<g.coverageOldDuplicate<<" "<<g.coverageNewDuplicate<<'\\n';
 }
'''+src[insert:]
Path(sys.argv[2]).write_text(src)
