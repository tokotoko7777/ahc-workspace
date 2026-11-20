#define AHC012_NO_MAIN
#include "../problems/ahc012/main.cpp"

static void check(const Input& in,const State& s){
  std::map<array<int,3>,int> cell;
  for(auto p:in.pos){
    array<int,3> rank{};
    for(int d=0;d<3;++d)
      for(int cut:s.cuts[d]) rank[d]+=p[d]>cut;
    ++cell[rank];
  }
  std::vector<int> bin(11);
  for(auto& [rank,n]:cell)if(n<=10)++bin[n];
  int score=0;
  for(int i=1;i<=10;++i)score+=std::min(bin[i],in.demand[i]);
  assert(s.value()==score);
}

int main(){
  std::mt19937 rng(120446);
  for(int test=0;test<80;++test){
    Input in;
    in.n=20+rng()%220;
    for(int d=1;d<=10;++d)in.demand[d]=rng()%13;
    for(int i=0;i<in.n;++i){
      int x=int(rng()%40)-20,y=int(rng()%40)-20;
      in.pos.push_back({x,y,x+y});
      for(int d=0;d<3;++d)in.order[d].emplace_back(in.pos.back()[d],i);
    }
    for(auto& v:in.order)std::sort(v.begin(),v.end());
    State s(in,Cuts{vector{-15,-7,-1,4,9,14},vector{-15,-7,-1,4,9,14},
                    test%2?vector{-21,-4,14}:vector<int>{}});
    check(in,s);
    for(int iter=0;iter<120;++iter){
      int d=rng()%(test%2?3:2), i=rng()%s.cuts[d].size(), old=s.cuts[d][i];
      int lo=i?s.cuts[d][i-1]+1:-37;
      int hi=i+1<int(s.cuts[d].size())?s.cuts[d][i+1]-1:37;
      int next=lo+int(rng()%(hi-lo+1));
      if(next==old)continue;
      s.move_cut(d,i,next);
      check(in,s);
      if(rng()%2){s.move_cut(d,i,old);check(in,s);}
    }
  }
}
