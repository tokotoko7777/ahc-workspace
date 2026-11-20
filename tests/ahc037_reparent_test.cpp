#define AHC037_TEST
#include "../problems/ahc037/main.cpp"

void check(const vector<Point>& targets,const vector<Operation>& ops){
  set<Point> seen{Root};
  map<Point,int> degree;
  for(auto [a,b]:ops){
    assert(seen.contains(a)&&precedes(a,b)&&a!=b);
    assert(seen.insert(b).second);
    ++degree[a];
  }
  set<Point> demand(targets.begin(),targets.end());
  for(auto p:targets)assert(seen.contains(p));
  for(auto p:seen)if(p!=Root&&!demand.contains(p))assert(degree[p]>=2);
}
int main(){
  mt19937 rng(3737);
  for(int t=0;t<800;++t){
    const int n=1+rng()%33;
    set<Point> p;
    while(int(p.size())<n)p.emplace(rng()%20,rng()%20);
    vector<Point> target(p.begin(),p.end());
    auto input=join(target);
    auto base=reparent(target,input,false),all=reparent(target,input,true);
    check(target,base);check(target,all);
    assert(cost(base)<=cost(input)&&cost(all)<=cost(input));
  }
}
