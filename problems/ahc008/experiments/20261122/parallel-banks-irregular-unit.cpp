#define main bank_main_unused
#include "parallel_banks_irregular.cpp"
#undef main
int main(){
 std::istringstream fixture("1\n11 7 1\n2\n11 11\n15 7\n");
 std::cin.rdbuf(fixture.rdbuf());
 Game g;char* a[]={nullptr};g.init(1,a);g.bankShift=4;g.turn=226;g.irregularMode=1;
 vector<int> in{pos(10,6),pos(10,7),pos(10,8),pos(11,6),pos(12,6)};
 int p=pos(10,9),q=pos(13,6);
 for(int u:in)for(int v:nb[u])if(v>=0&&v!=p&&v!=q&&find(in.begin(),in.end(),v)==in.end())g.wall[v]=1;
 g.prep();
 auto actual=g.irregularCut("..");
 assert(actual=="lu");
 g.pets[0].p=pos(10,8);g.prep();assert(g.irregularCut("..")=="..");
 std::cout<<"supported five-cell irregular pocket; two builders, observed pet blocker veto verified\n";
}
