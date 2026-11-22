#define main ahc008_entry
#include "solve.cpp"
#undef main
int main(){
 using namespace lastcap;
 const auto P=[](int r,int c){return r*30+c;};
 Board wall{};
 for(int c=0;c<30;c++)if(c!=7&&c!=15&&c!=23)wall[P(5,c)]=1;
 cordon::Front f;
 f.start=169;f.attempts=1;f.initialSize=873;f.kept=720;
 for(int p=P(6,0);p<900;p++)f.exterior[p]=1;
 f.cut={P(5,7),P(5,15),P(5,23)};
 f.gates={P(5,7),P(5,23)};f.crew={0,1};
 std::vector<int> humans{P(6,7),P(6,23),P(6,15),P(7,20),P(7,0)};
 std::vector<lastcap::Pet> pets{{P(1,10),2}};
 auto q=f.act(170,wall,humans,pets);
 assert(q&&*q=="..u.."&&f.shellWalls==1);
 wall[P(5,15)]=1;
 pets[0].p=P(8,17);
 q=f.act(171,wall,humans,pets);
 assert(q&&*q=="....."&&f.heldOutside==1);
 pets[0].p=P(1,10);
 q=f.act(172,wall,humans,pets);
 assert(q&&*q=="uu..."&&f.closes==1);
 wall[P(5,7)]=wall[P(5,23)]=1;
 lastcap::BFS b;
 assert(b.run(humans[0],Planner::walkable(wall))>=300 && b.d[pets[0].p]==-1);
}
