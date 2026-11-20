#define AHC016_NO_MAIN
#define AHC016_SIZE_TEST
#include "../problems/ahc016/main.cpp"

int main() {
  constexpr uint64_t initial=0x118811ee023ULL;
  for(int n:{8,19,37})for(double p:{0.0,0.13,0.4}){
    DegreeCode book(12,p,n);
    int wrong=0;
    NoiseRng rng{initial};
    for(int rep=0;rep<16;++rep)for(int label=0;label<int(book.code.size());++label){
      auto message=book.code[label];
      for(char& bit:message)if(rng.next()<lround(p*4294967296.0))
        bit=(bit=='1'?'0':'1');
      wrong+=(book.decode(message)!=label);
    }
    auto estimate=error_estimate(book,p,16,initial,
                                 SizeClock::now()+std::chrono::seconds(1));
    assert(estimate && *estimate==double(wrong)/(16*book.code.size()));
    assert(!error_estimate(book,p,16,initial,SizeClock::now()));
  }
}
