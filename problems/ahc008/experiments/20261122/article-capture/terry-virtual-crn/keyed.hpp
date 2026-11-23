#pragma once
#include <cassert>
#include <cstdint>
namespace keyed_crn {
constexpr uint64_t mix(uint64_t x){
 x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;
 x=(x^(x>>27))*0x94d049bb133111ebULL;
 return x^(x>>31);
}
constexpr uint64_t bits(int t,int pet,int event,uint64_t retry){
 return (retry<<16)|(uint64_t(t-275)<<9)|(uint64_t(pet)<<3)|uint64_t(event);
}
struct Words{
 uint64_t seed,slot=0,retry=0;
 void begin(int t,int pet,int event){
  assert(t>=275&&t<300&&pet>=0&&pet<64&&event>=0&&event<8);
  slot=bits(t,pet,event,0);retry=0;
 }
 uint32_t next32(){
  assert(retry<(uint64_t(1)<<48));
  return uint32_t(mix(seed^mix(slot|(retry++<<16)))>>32);
 }
};
}
