#define main fixture_main
#include "solve.cpp"
#undef main
int main(){
 using namespace lastcap;
 auto at=[](int r,int c){return r*30+c;};
 Board wall{},main;
 for(int r=0;r<=4;r++)for(int c=0;c<=4;c++)
  if(r==0||r==4||c==0||c==4)wall[at(r,c)]=1;
 int x=at(1,4),y=at(4,2);
 wall[x]=wall[y]=0;
 std::vector<int> humans{at(1,5),at(5,2),at(8,2),at(8,3),at(8,4)};
 std::vector<lastcap::Pet> pets{{at(2,2),1}};
 auto open=Planner::walkable(wall);
 BFS nav;int size=nav.run(humans.front(),open);
 for(int p=0;p<900;p++)main[p]=nav.d[p]>=0;
 Planner p;
 auto s=p.offsetFrontier(245,wall,main,size,humans,pets,{});
 assert(s.cut==std::vector<int>({x,y}));
 assert(s.trapped==1&&s.kept==size-11&&s.cost==1);
 auto move=p.plan(245,wall,humans,pets,{},{});
 assert(move&&(*move)[0]=='l'&&(*move)[1]=='u'&&p.offsetAttempts==1);
 Board protect{};protect[x]=1;
 auto protectedResult=p.offsetFrontier(245,wall,main,size,humans,pets,protect);
 assert(std::find(protectedResult.cut.begin(),protectedResult.cut.end(),x)==protectedResult.cut.end());
 auto five=humans;five.pop_back();
 assert(p.offsetFrontier(245,wall,main,size,five,pets,{}).value<0);
 assert(p.offsetFrontier(234,wall,main,size,humans,pets,{}).value<0);
}
