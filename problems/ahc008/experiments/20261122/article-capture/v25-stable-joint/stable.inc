 struct StableJoint {
  Pen pen;
  std::vector<int> issued;
  int last=-100,tube=-1,a=-1,b=-1,rail=-1,pet=-1,dry=0;
 } stableJoint;
 bool stableJointEnabled=true;
 int stableContinued=0;
 // A retained moving-only turn needs an issued crew move that advances
 // an actually legal pending wall from a real exterior stance within two
 // steps (including this turn). Ignore a goal if another wall blocks the walk.
 bool stableJointWalk(const bitset<V>& ended,const vector<bool>& crew,const Pen& z)const{
  for(int i=0;i<5;i++)if(crew[i]&&isupper(act[i])){
   int from=humans[i],to=nb[from][find(MOVE,MOVE+4,act[i])-MOVE];
   if(to<0||ended[to])continue;
   for(int q:z.wall)if(!wall[q]&&!block[q]&&find(nb[from].begin(),nb[from].end(),q)==nb[from].end())for(int stance:nb[q]){
    if(stance<0||ended[stance]||stance==from)continue;
    if(stance==to)return true;
    if(find(nb[to].begin(),nb[to].end(),stance)!=nb[to].end()&&
       find(nb[from].begin(),nb[from].end(),stance)==nb[from].end())return true;
   }
  }
  return false;
 }
