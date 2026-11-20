#define AHC012_TARGET_CUT
#define AHC012_NO_MAIN
#include "../problems/ahc012/main.cpp"

int main() {
  std::mt19937 rng(120568);
  for(int a=0;a<=4;++a)for(int b=-4;b<=4;++b)if(std::gcd(a,b)==1){
    for(int i=0;i<1600;++i){
      const int x=int(rng()%20001)-10000,y=int(rng()%20001)-10000;
      const int projection=a*x+b*y;
      for(int c:{projection,projection-1,projection+1,2*projection,-45000,45000}){
        auto [u,v,p,q]=TargetCut{a,b,c}.endpoints();
        assert(std::max({abs(u),abs(v),abs(p),abs(q)})<=1000000000);
        const int64_t side=int64_t(p-u)*(y-v)-int64_t(q-v)*(x-u);
        assert(side!=0 && (side>0)==(projection>c));
      }
    }
  }
}
