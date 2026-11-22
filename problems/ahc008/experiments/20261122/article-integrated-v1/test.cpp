#define main integrated_entry
#include "integrated.cpp"
#undef main
int main(){
 Game g;
 for(int p=0;p<900;p++){
  int r=p/30,c=p%30;
  nb[p]={r?p-30:-1,r<29?p+30:-1,c?p-1:-1,c<29?p+1:-1};
 }
 g.humans={0,1,2,3,4};g.pets={{465,1},{465,2},{465,3},{465,4},{465,5}};
 g.setupObservedMotion();g.observedMotionGraph();
 for(int i=0;i<5;i++){
  Game::Forecast f{};g.forecastPet(i,10,f);
  for(int h=0;h<=10;h++){
   auto &v=f[h];
   assert(abs(accumulate(v.begin(),v.end(),0.)-1.)<2e-5);
   if(i==1||i==3||i==4)for(int p=0;p<900;p++)
    if(((p/30+p%30)^(465/30+465%30))&1)assert(v[p]==0.);
  }
 }
 vector<vector<Game::BankJob>> opts={{{0,0,1,1,3.},{0,2,3,1,4.}},{{1,2,3,1,9.},{1,2,4,1,6.}}};
 auto common=g.packBankJobs(opts,2);
 assert(common.size()==2);
 double val=0;
 for(auto j:common)val+=j.value;
 assert(val==12.);
 assert(g.packBankJobs(opts,1)[0].value==9.);
}
