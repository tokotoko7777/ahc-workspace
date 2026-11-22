#define main ahc008_entry
#include "solve.cpp"
#undef main
int main() {
 Game g;
 g.humans.resize(4);g.pets.resize(2);
 using J=Game::BankJob;
 vector<vector<J>> jobs{{{0,0,1,2,.8}},{{1,2,3,2,.6}},{{2,2,3,2,.45}},{{3,0,2,2,.3}}};
 vector<array<Game::PetCredit,Game::Horizon+1>> p(4);
 p[0][2][0]=p[1][2][0]=1;
 p[2][2][1]=p[3][2][1]=1;
 const auto baseline=g.packBankJobs(jobs,2);
 assert(baseline.size()==2 && baseline[0].tube==1 && baseline[1].tube==0);
 auto got=g.diversifyBankJobs(jobs,baseline,p);
 assert(got.size()==2 && got[0].tube==2 && got[1].tube==0);
 assert(g.coveragePlans==1 && g.coverageChanges==1 && g.coverageOldDuplicate>.59 && g.coverageNewDuplicate<1e-8);
 assert(g.diversifyBankJobs(jobs,{baseline[0]},{}).front().tube==1);
}
