#define main interactive_test
#include "integrated.cpp"
#undef main
int main(){
 Game g;
 for(int p=0;p<900;p++) {
  int r=p/30,c=p%30;
  nb[p]={r?p-30:-1,r<29?p+30:-1,c?p-1:-1,c<29?p+1:-1};
 }
 g.humans={0,1,2,3,4};g.pets={{30*15+15,5},{30*15+15,2}};
 g.setupObservedMotion();g.observedMotionGraph();
 g.catHead[0]=3;
 g.catPosterior[0].target[30*10+15]=1.f;g.catPosterior[0].reset=0.;
 Game::Forecast f{};g.forecastPet(0,6,f);
 auto old=f;g.replaceShortCat(0,3,f);
 assert(f[1][30*13+15]==.25f&&f[1][30*15+17]==0.f);
 assert(old[1][30*15+17]>0.f);
 assert(f[0]==old[0]&&f[4]==old[4]&&f[5]==old[5]&&f[6]==old[6]);
 for(int h=1;h<=3;h++)assert(abs(accumulate(f[h].begin(),f[h].end(),0.)-1.)<1e-6);
 Game::Forecast pig{};g.forecastPet(1,5,pig);
 const auto same=pig;g.replaceShortCat(1,3,pig);assert(same==pig);
}
