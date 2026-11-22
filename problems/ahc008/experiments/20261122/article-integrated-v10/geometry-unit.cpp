#define main interactive_test
#include "integrated.cpp"
#undef main
int main(){
 for(int m=5;m<=10;m++){
  Game g;g.bankMode=5;g.narrowRescue=true;g.humans.assign(m,0);g.makeBanks();
  bool small=m<=7;
  assert(g.bankN==(small?2:3));
  assert(g.banks.size()==(small?26:36));
  assert(g.bankWalls.size()==(small?112:156));
 }
}
