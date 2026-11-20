#define AHC042_TEST
#include "../problems/ahc042/main.cpp"

int main(){
  using Whole=PackedLineShift<32>;
  assert(Whole::shifted(1u,31,true)==(1u<<31));
  assert(Whole::shifted(0xffffffffu,32,true)==0);
  mt19937 rng(420042);
  for(int t=0;t<2600;++t){
    Board b;
    vector<string> g(N,string(N,'.'));
    for(int y=0;y<N;++y)for(int x=0;x<N;++x){
      int k=rng()%7;
      if(k==0){b.x[y]|=1u<<x;g[y][x]='x';}
      if(k==1){b.o[y]|=1u<<x;g[y][x]='o';}
    }
    const auto bits=[](const auto& rows){
      int n=0;for(auto x:rows)n+=popcount(x);return n;
    };
    preps(b,[&](Choice choice,int removed,const Board& after){
      auto semi=apply(b,choice.prep);
      assert(bits(semi.x)==bits(b.x)&&bits(semi.o)==bits(b.o));
      assert(bits(after.o)==bits(b.o));
      assert(bits(b.x)-bits(after.x)==removed&&removed>0);
      assert(apply(semi,choice.eject)==after);
    });
    Macro m{uint8_t(rng()%2),uint8_t(rng()%2),uint8_t(rng()%N),uint8_t(1+rng()%N),0};
    m.back=rng()%(m.out+1);
    auto move=[&](bool sign){
      int i=m.line;
      string l(N,'.');
      for(int z=0;z<N;++z){
        int old=z+(sign?-1:1);
        if(0<=old&&old<N)l[z]=m.axis?g[old][i]:g[i][old];
      }
      for(int z=0;z<N;++z)(m.axis?g[z][i]:g[i][z])=l[z];
    };
    for(int i=0;i<m.out;++i)move(m.sign);
    for(int i=0;i<m.back;++i)move(!m.sign);
    auto out=apply(b,m);
    for(int y=0;y<N;++y)for(int x=0;x<N;++x){
      assert(bool(out.x[y]>>x&1u)==(g[y][x]=='x'));
      assert(bool(out.o[y]>>x&1u)==(g[y][x]=='o'));
    }
  }
}
