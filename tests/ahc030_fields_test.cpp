#define AHC030_TEST
#include "../problems/ahc030/main.cpp"

int main(){
  mt19937 rng(303031);
  for(int trial=0;trial<600;++trial){
    Solver s;
    s.n=4;s.L=16;s.m=2+rng()%4;s.shapes.resize(s.m);
    s.observed.fill(-1);
    array<int,Limit> ground{};
    vector<int> truth;
    for(auto& shape:s.shapes){
      int size=2+rng()%6,count=2+rng()%9;
      for(int p=0;p<count;++p){
        Placement pos;
        while(int(pos.cells.size())<size){
          int i=rng()%s.L;
          if(pos.mask[i])continue;
          pos.mask[i]=1;pos.cells.push_back(i);++shape.cover[i];
        }
        shape.pos.push_back(std::move(pos));
      }
      shape.active_count=count;
      truth.push_back(rng()%count);
      for(int i:shape.pos[truth.back()].cells)++ground[i];
      s.total+=size;
    }
    vector<int> order(s.L);iota(order.begin(),order.end(),0);
    shuffle(order.begin(),order.end(),rng);
    for(int i:order){
      s.observed[i]=ground[i];
      s.propagate(Clock::time_point::max());
      for(int k=0;k<s.m;++k){
        auto& shape=s.shapes[k];
        assert(shape.pos[truth[k]].active);
        array<int,Limit> brute{};
        int count=0;
        for(auto& pos:shape.pos)if(pos.active){
          ++count;
          for(int v=0;v<s.L;++v)brute[v]+=pos.mask[v];
        }
        assert(count==shape.active_count&&brute==shape.cover);
      }
      for(int j=0;j<s.L;++j)
        assert(s.lower[j]<=ground[j]&&ground[j]<=s.upper[j]);
    }
  }
}
