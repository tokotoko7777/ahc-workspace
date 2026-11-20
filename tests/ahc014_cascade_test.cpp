#define AHC014_NO_MAIN
#define AHC014_CASCADE_TEST
#include "../problems/ahc014/main.cpp"

int main(){
  for(int seed=1;seed<=100;++seed){
    int n=8+seed%11;
    Rng rng{uint64_t(seed)*2654435761u};
    State initial(n);
    for(int x=1;x+1<n;++x)for(int y=1;y+1<n;++y)
      if(rng.next()%4==0)initial.insert({x,y});
    State full=initial;
    std::vector<double> cost(2*n+1,1.0),weight(n*n,1.0);
    while(full.answer.size()<60){
      auto move=choose(full,cost,weight,0.5,rng);
      if(!move)break;
      full.apply(*move);
    }
    const auto len=full.answer.size();
    if(!len)continue;
    std::vector<int> owner(n*n,-1);
    for(int i=0;i<int(len);++i){
      auto q=full.answer[i].corners()[0];
      owner[q.x*n+q.y]=i;
    }
    for(int remove=0;remove<int(len);++remove){
      std::vector<bool> invalid(len);
      invalid[remove]=true;
      for(int i=remove+1;i<int(len);++i){
        auto [p,a,b,c]=full.answer[i].corners();
        for(auto q:{a,b,c}){
          int parent=owner[q.x*n+q.y];
          assert(parent<i);
          invalid[i]=invalid[i]||(parent>=0&&invalid[parent]);
        }
      }
      auto keep=replay_without(initial,full.answer,remove);
      int k=0;
      int value=initial.value;
      for(int i=0;i<int(len);++i)if(!invalid[i]){
        auto p=full.answer[i].corners()[0];
        auto q=keep.answer.at(k++).corners()[0];
        assert(p.x==q.x&&p.y==q.y);
        value+=initial.weight(p);
      }
      assert(k==int(keep.answer.size())&&value==keep.value);
    }
  }
}
