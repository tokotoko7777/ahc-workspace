#define AHC026_NO_MAIN
#include "../problems/ahc026/main.cpp"

int main(){
  std::mt19937 rng(266501);
  for(int trial=0;trial<180;++trial){
    StackMap b;
    std::array<std::vector<int>,M> ref;
    std::iota(b.data.begin(),b.data.end(),uint8_t(1));
    std::shuffle(b.data.begin(),b.data.end(),rng);
    b.size.fill(N/M);
    for(int i=0;i<M;++i)ref[i].assign(b.data.begin()+i*(N/M),b.data.begin()+(i+1)*(N/M));
    auto check=[&]{
      const auto p=b.profile();
      int total=0;
      for(int i=0;i<M;++i){
        assert(total==p[i].begin&&b.size[i]==ref[i].size());
        for(int x:ref[i])assert(b.data[total++]==x);
        assert(p[i].end==total);
        assert(p[i].minimum==(ref[i].empty()?N+1:*min_element(ref[i].begin(),ref[i].end())));
      }
      assert(total==b.left);
    };
    for(int t=0;t<250;++t){
      int src=rng()%M;
      if(ref[src].empty())continue;
      auto p=b.profile();
      if(rng()%4==0){
        b.remove_top(src,p[src].end-1);
        ref[src].pop_back();
      }else{
        int dst=(src+1+rng()%(M-1))%M;
        int offset=rng()%ref[src].size();
        auto moved=ref[src].size()-offset;
        assert(b.move(src,p[src].begin+offset,dst,p)==int(moved)+1);
        ref[dst].insert(ref[dst].end(),ref[src].begin()+offset,ref[src].end());
        ref[src].erase(ref[src].begin()+offset,ref[src].end());
      }
      check();
    }
  }
}
