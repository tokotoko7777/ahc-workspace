from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
a=s.index('namespace lastcap {');b=s.index('namespace terry_maps::weights',a)
t=s[a:b]
def patch(p,q):
 global t
 assert t.count(p)==1,p
 t=t.replace(p,q)
patch('''   int dur=far+2+int((z.cut.size()*2+H-1)/H)+left/2;
   if(dur>=horizon)continue;
   z.cost=dur;
   z.value=(double(z.kept*(1<<z.trapped))-size)/(1.+.045*dur+.034*z.cut.size()+.012*near);''','''   int dur=far+2+int((z.cut.size()*2+H-1)/H)+left/2;
   int exposure=0;
   if(H==5&&z.cut.size()<=5){
    // Certify distinct builders and compare all distances inside the retained
    // human component, rather than independently assigning nearest humans.
    std::array<BFS,5> staff;
    for(int i=0;i<H;++i)staff[i].run(humans[i],z.can);
    int d[5][5];
    for(int j=0;j<int(z.cut.size());++j)for(int i=0;i<H;++i){
     d[j][i]=INF;
     for(int q:neighbors(z.cut[j]))if(q>=0&&staff[i].d[q]>=0)
      d[j][i]=std::min(d[j][i],staff[i].d[q]);
    }
    std::array<int,32> dp;dp.fill(INF);dp[0]=0;
    int simult=INF;
    for(unsigned used=0;used<32;++used)if(dp[used]<INF){
     int j=std::popcount(used);
     if(j==int(z.cut.size())){simult=std::min(simult,dp[used]);continue;}
     for(int i=0;i<H;++i)if(!(used>>i&1)&&d[j][i]<INF){
      unsigned u=used|(1u<<i);
      dp[u]=std::min(dp[u],std::max(dp[used],d[j][i]));
     }
    }
    if(simult<INF){
     dur=1+simult;
     int allSafe=INF;
     for(auto pet:pets)if(main[pet.p]&&!z.can[pet.p]){
      int slack=INF;
      for(int p:z.cut)slack=std::min(slack,dist1(p,pet.p)-1);
      int speed=pet.t==1?1:pet.t==3?3:2;
      allSafe=std::min(allSafe,std::max(0,slack)/speed);
     }
     exposure=std::max(0,simult-allSafe);
    }
   }
   if(dur>=horizon)continue;
   z.cost=dur;
   z.value=(double(z.kept*(1<<z.trapped))-size)/(1.+.045*dur+.034*z.cut.size()+.012*near+.16*exposure);''')
Path(sys.argv[2]).write_text(s[:a]+t+s[b:])
