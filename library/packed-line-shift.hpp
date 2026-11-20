#pragma once
#include <array>
#include <cassert>
#include <cstdint>

// Small board packed as one uint32_t per row. Bits outside N are zero.
template<int N>
struct PackedLineShift {
  static_assert(1<=N && N<=32);
  using Board=std::array<std::uint32_t,N>;
  static constexpr std::uint32_t mask=std::uint32_t(-1)>>(32-N);

  static std::uint32_t get(const Board& rows,bool column,int index){
    assert(0<=index&&index<N);
    if(!column)return rows[index];
    std::uint32_t bits=0;
    for(int i=0;i<N;++i)bits|=((rows[i]>>index)&1u)<<i;
    return bits;
  }
  static void put(Board& rows,bool column,int index,std::uint32_t bits){
    assert(0<=index&&index<N);
    bits&=mask;
    if(!column){rows[index]=bits;return;}
    for(int i=0;i<N;++i)
      rows[i]=(rows[i]&~(1u<<index))|(((bits>>i)&1u)<<index);
  }
  static std::uint32_t shifted(std::uint32_t bits,int distance,bool positive){
    assert(distance>=0);
    if(distance>=N)return 0;
    return positive?(bits<<distance)&mask:(bits&mask)>>distance;
  }
};
