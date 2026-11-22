#define main fixture_main
#include "solve.cpp"
#undef main
int main(){
 for(int p=0;p<900;p++){
  int r=p/30,c=p%30;
  nb[p]={r?p-30:-1,r<29?p+30:-1,c?p-1:-1,c<29?p+1:-1};
 }
 Game g;auto at=[](int r,int c){return r*30+c;};
 g.humans={at(5,5),at(7,8),at(11,12),at(15,15),at(15,18)};
 g.pets={{at(14,14),1},{at(2,2),2}};
 for(int i=0;i<900;i++)g.group[i]=0;
 g.currentCt[0]=885;
 for(int r=0;r<=4;r++)for(int c=0;c<=4;c++)if(r==0||r==4||c==0||c==4)g.wall[at(r,c)]=true;
 int door=at(1,4);g.wall[door]=false;
 Pen z;z.wall={door};
 bitset<900> path;path[at(5,5)]=true;
 assert(g.jointPocketCert(z,0,path));
 assert(!g.jointPocketCert(z,1,path));
 path[door]=true;assert(!g.jointPocketCert(z,0,path));
 path[door]=false;
 assert(g.jointPocketCert(z,0,path,1));
 assert(!g.jointPocketCert(z,0,path,0));
 bitset<900> end;
 for(int i=0;i<900;i++)end[i]=g.wall[i];
 end[door]=true;
 g.humans[0]=at(2,5);
 g.act="U....";
 vector<bool> crew(5);crew[0]=true;
 assert(g.stableJointWalk(end,crew,z));
 g.block[door]=1;assert(!g.stableJointWalk(end,crew,z));
 g.block[door]=0;
 g.act="D....";assert(!g.stableJointWalk(end,crew,z));
 g.turn=205;g.stableJoint.last=204;
 assert(!g.jointRescue()&&g.stableJoint.last<0);
 g.jointEnabled=false;assert(!g.jointRescue());
}
