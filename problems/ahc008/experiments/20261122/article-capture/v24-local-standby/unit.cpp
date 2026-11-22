#define main fixture_main
#include "solve.cpp"
#undef main
int main(){
 for(int r=0;r<30;r++)for(int c=0;c<30;c++){
  int p=r*30+c;
  // This base orders U,D,L,R.
  nb[p]={r?p-30:-1,r<29?p+30:-1,c?p-1:-1,c<29?p+1:-1};
 }
 Game g;
 auto at=[](int r,int c){return 30*r+c;};
 g.humans={at(11,5),at(11,12),at(12,5),at(13,5),at(0,0)};
 g.pets={{at(9,8),1}};
 g.terryRoads={at(15,5),at(15,12)};
 g.banks.push_back({0,9,at(9,5),at(9,6),at(9,11),at(9,12),
   {at(9,7),at(9,8),at(9,9),at(9,10)},{}});
 g.act="...L.";
 std::bitset<900> used,build,moves;used[3]=true;
 g.localStandby({1},used,build,moves);
 // Same animal attracts a guard on each side; only one per aisle.
 assert(g.localStandbyMatched==2&&g.localStandbyMoves==2);
 assert(g.act=="UU.L."&&used[0]&&used[1]&&used[2]&&used[3]&&!used[4]);
 assert(moves[at(10,5)]&&moves[at(10,12)]);
 g.localStandbyEnabled=false;
 std::bitset<900> off,mv;g.act=".....";
 g.localStandby({1},off,build,mv);
 assert(off.none()&&mv.none()&&g.act==".....");
}
