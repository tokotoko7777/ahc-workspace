#include <cassert>
#include <random>
#include "../library/bounded-layer-shortest-path.hpp"

int main(){
  std::mt19937 rng(240552);
  for(int t=0;t<1200;++t){
    int h=1+rng()%6,w=1+rng()%5,jump=rng()%4;
    std::vector<std::optional<int>> nodes(h*w),edges(h*w*w);
    auto connect=[&](int y,int a,int b){return edges[(y*w+a)*w+b];};
    for(auto* list:{&nodes,&edges})for(auto& x:*list)
      if(rng()%5)x=int(rng()%31)-15;
    auto computed=bounded_layer_shortest_path<int>(
      h,w,jump,[&](int y,int x){return nodes[y*w+x];},connect);
    std::optional<int> answer;
    auto dfs=[&](auto&& self,int y,int old,int cost)->void{
      if(y==h){if(!answer||cost<*answer)answer=cost;return;}
      for(int i=0;i<w;++i)if(nodes[y*w+i]){
        if(!y)self(self,y+1,i,cost+*nodes[y*w+i]);
        else if(std::abs(i-old)<=jump)
          if(auto d=connect(y,old,i))self(self,y+1,i,cost+*nodes[y*w+i]+*d);
      }
    };
    dfs(dfs,0,-1,0);
    assert(bool(answer)==bool(computed));
    if(computed){
      assert(*answer==computed->cost);
      assert(int(computed->states.size())==h);
      int score=0;
      for(int y=0;y<h;++y){
        int s=computed->states[y],p=y?computed->states[y-1]:-1;
        assert(s>=0&&s<w&&nodes[y*w+s]);
        score+=*nodes[y*w+s];
        if(y){
          assert(std::abs(s-p)<=jump&&connect(y,p,s));
          score+=*connect(y,p,s);
        }
      }
      assert(score==*answer);
    }
  }
}
