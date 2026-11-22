from pathlib import Path
import sys,hashlib
p=Path(sys.argv[1]); assert hashlib.sha256(p.read_bytes()).hexdigest()=='46f210cc5dd50b8a6088724861ae5c2b044d08edb109a891ee217352730fcb84'
s=p.read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,a[:120]
 s=s.replace(a,b)
a='''   double best=0.;
   int limit=min(lineMax,(299-turn)*max(1,int(hm)-1));'''
rep(a,'''   double best=0.;
   struct EarlyCandidate{double value;int id,begin,end,eta;vector<int> cells;};
   vector<EarlyCandidate> earlyOptions;
   int limit=min(lineMax,(299-turn)*max(1,int(hm)-1));''')
a='''      if(v>best){best=v;lineId=j;lineBase=i;line.at=e;lineH=root;lineStart=turn;toBuild=move(w);}
     }
    }
   }
#ifdef AHC008_WIGGLE'''
rep(a,'''      if(earlyShort&&v>0){
       auto pos=lower_bound(earlyOptions.begin(),earlyOptions.end(),v,
          [](const EarlyCandidate& a,double b){return a.value>b;});
       if(earlyOptions.size()<4||pos!=earlyOptions.end()){
        earlyOptions.insert(pos,{v,j,i,e,eta,w});
        if(earlyOptions.size()>4)earlyOptions.pop_back();
       }
      }
      if(v>best){best=v;lineId=j;lineBase=i;line.at=e;lineH=root;lineStart=turn;toBuild=move(w);}
     }
    }
   }
   if(earlyShort&&earlyOptions.size()>1&&!overBudget()){
    if(!postReady)observedMotionGraph();
    auto prior=mcDogs();
    int maxEta=0;for(const auto& z:earlyOptions)maxEta=max(maxEta,z.eta);
    const auto sample=terry_mc::sample_paths(mcState(),terrySim,24,maxEta,&prior);
    int winner=-1;
    double value=-1.;
    for(int i=0;i<int(earlyOptions.size());++i){
     const auto& z=earlyOptions[i];
     const auto [staticGain,side]=testLine(z.cells,root,true,0,areaReq);
     if(side<0||bn-lanimals[side]<2)continue;
     int persistent=0;
     for(int t=0;t<sample.trials;++t){
      const auto& ps=sample.at(t,z.eta);
      int hit=0;
      for(int j=0;j<int(pets.size());++j)if(group[pets[j].p]==root
         &&linTag[ps[j]]==linStamp&&linLabel[ps[j]]!=side)++hit;
      persistent+=hit>=2;
     }
     double adjusted=z.value*(.2+.8*persistent/max(1,sample.trials));
     if(adjusted>value)winner=i,value=adjusted;
    }
    if(winner>=0){
     const auto& z=earlyOptions[winner];
     lineId=z.id;lineBase=z.begin;separators[z.id].at=z.end;lineH=root;lineStart=turn;toBuild=z.cells;
    }
   }
#ifdef AHC008_WIGGLE''')
Path(sys.argv[2]).write_text(s)
