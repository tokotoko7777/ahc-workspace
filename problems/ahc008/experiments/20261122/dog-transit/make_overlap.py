from pathlib import Path
s=Path('transit_cap4.cpp').read_text()
s=s.replace(' vector<int> pp(g.pets.size()), tt;',''' vector<int> pp(g.pets.size()), tt;
 vector<unsigned char> released(g.humans.size());
 int scaffold_walls=0,scaffold_moves=0;''')
old='  if(g.act.empty())g.act=g.terminal(g.tacticalGate(g.repair(g.plan())));'
s=s.replace(old,old+'''
  else if(!trans.closed&&trans.guard>=0){
   // Protect the near guard and two intact bait/guard roles at the far end.
   int keep=2;
   int gnear=trans.guard;
   if(g.humans[gnear]==trans.z.a)for(int j=0;j<(int)g.humans.size();j++)
    if(j!=gnear&&!released[j]&&g.humans[j]==trans.z.b){
     if(keep>0)--keep;
     else released[j]=1;
    }
   if(count(released.begin(),released.end(),1)){
    string original=g.act;
    g.prep();
    if(!g.roomReady)g.createRooms();
    for(int j=0;j<(int)g.humans.size();j++)
     g.act[j]=released[j]?(original[j]>='a'?original[j]:'.'):'#';
    vector<pair<int,int>> incomplete;
    auto nearCorridor=[&](int p) {
      if(md(p,trans.z.a)<=2||md(p,trans.z.b)<=2)return true;
      for(int q:trans.z.lane)if(md(p,q)<=2)return true;
      return false;
    };
    int active=g.group[g.humans[gnear]];
    for(int p:g.rwall)if(!g.wall[p]&&g.group[p]==active&&!nearCorridor(p)){
      int near=99;
      for(auto pet:g.pets)if(g.group[pet.p]==active)near=min(near,md(p,pet.p));
      incomplete.emplace_back(p,60+min(9,near)); // identical cap4 task priority
    }
    auto overlay=g.roomBuild(incomplete,0,false);
    bitset<V> walls,moves;
    g.act=original;
    for(int j=0;j<(int)g.humans.size();j++)if(released[j]&&g.act[j]<'a')g.act[j]='.';
    for(int j=0;j<(int)g.humans.size();j++){
      if(g.act[j]>='a')walls[nb[g.humans[j]][find(BUILD,BUILD+4,g.act[j])-BUILD]]=true;
      else if(g.act[j]!='.')moves[nb[g.humans[j]][find(MOVE,MOVE+4,g.act[j])-MOVE]]=true;
    }
    for(int j=0;j<(int)g.humans.size();j++)if(released[j]&&g.act[j]=='.'){
      char c=overlay[j];
      if(c>='a'){
       int p=nb[g.humans[j]][find(BUILD,BUILD+4,c)-BUILD];
       if(!moves[p]&&!walls[p]){g.act[j]=c; walls[p]=1;++scaffold_walls;}
      }else if(c!='.'){
       int p=nb[g.humans[j]][find(MOVE,MOVE+4,c)-MOVE];
       if(!walls[p]){g.act[j]=c; moves[p]=1;++scaffold_moves;}
      }
    }
   }
  }''')
s=s.replace(' if(getenv("TRANSIT_DIAG"))cerr<<',' if(getenv("SCAFFOLD_DIAG"))cerr<<"SCAFFOLD "<<scaffold_walls<<" "<<scaffold_moves<<"\\n";\n if(getenv("TRANSIT_DIAG"))cerr<<')
Path('transit_overlap.cpp').write_text(s)
