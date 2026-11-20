#define AHC022_NO_MAIN
#define AHC022_ENERGY_TEST
#include "../problems/ahc022/main.cpp"

int main(){
  std::mt19937 rng(220350);
  for(int test=0;test<1400;++test){
    int n=2+rng()%48, d=4+rng()%20;
    vector<unsigned> a(n),changed(n);
    for(auto& v:a)v=rng()&((1u<<d)-1);
    int initial_min=d;
    double initial=0;
    for(int i=0;i<n;++i)for(int j=0;j<i;++j){
      int h=popcount(a[i]^a[j]);
      initial+=separation_penalty(h);
      initial_min=min(initial_min,h);
    }
    for(auto& v:changed)if(rng()%4==0) v=1u<<(rng()%d);
    auto result=check_word_edit(a,changed,initial_min);
    for(int i=0;i<n;++i)a[i]^=changed[i];
    double after=0;
    int actual_min=d;
    for(int i=0;i<n;++i)for(int j=0;j<i;++j){
      int h=popcount(a[i]^a[j]);
      after+=separation_penalty(h);
      actual_min=min(actual_min,h);
    }
    assert(result.valid==(actual_min>=initial_min));
    if(result.valid)assert(abs(result.delta-(after-initial))<1e-7);
  }
}
