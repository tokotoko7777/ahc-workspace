#include "keyed.hpp"
#include <array>
#include <bit>
#include <set>
#include <iostream>
int main(){
 for(unsigned n=1;n<=31;++n){
  unsigned z=(n<<(std::countl_zero(n)-24))-1;
  std::array<int,32> count{};
  for(unsigned w=0;w<256;++w){unsigned q=w*n;if((q&255)<=z)++count[q>>8];}
  assert(count[0]>0);
  for(unsigned j=0;j<n;++j)assert(count[j]==count[0]);
 }
 std::set<uint64_t> keys;
 for(int t=275;t<300;++t)for(int p=0;p<20;++p)for(int e=0;e<6;++e)
 for(int retry=0;retry<8;++retry)assert(keys.insert(keyed_crn::bits(t,p,e,retry)).second);
 keyed_crn::Words a{77},b{77};
 a.begin(275,0,0);for(int i=0;i<128;++i)a.next32();
 a.begin(277,3,2);b.begin(277,3,2);assert(a.next32()==b.next32());
 std::cout<<"bounded reduced-width law and isolated event slots passed\n";
}
