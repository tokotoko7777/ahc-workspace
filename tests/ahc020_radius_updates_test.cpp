#define AHC020_NO_MAIN
#include "../problems/ahc020/main.cpp"

int main(){
  std::mt19937 rng(200578);
  for(int trial=0;trial<60;++trial){
    Instance in{};
    in.n=5+rng()%20;
    in.k=10+rng()%100;
    in.radius.assign(in.n,vector<int>(in.k));
    in.reaches.resize(in.n);
    for(int i=0;i<in.n;++i){
      for(int j=0;j<in.k;++j){
        int r=1+rng()%36;
        in.radius[i][j]=r;
        in.reaches[i].push_back({r,j});
      }
      sort(in.reaches[i].begin(),in.reaches[i].end());
    }
    Radio s(in);
    auto check=[&]{
      Int cost=0;
      vector<int> hits(in.k);
      for(int i=0;i<in.n;++i){
        cost+=Int(s.power[i])*s.power[i];
        for(int j=0;j<in.k;++j)hits[j]+=in.radius[i][j]<=s.power[i];
      }
      assert(cost==s.cost && hits==s.covered);
    };
    for(int round=0;round<250;++round){
      int i=rng()%in.n, was=s.power[i];
      s.set(i,rng()%40);
      check();
      if(rng()%2)s.set(i,was),check();
    }
  }
}
