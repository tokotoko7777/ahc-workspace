#pragma once
#include <array>
#include <cassert>

// Reserve of each stack layer after returning directly home. Extending
// by a new top item changes earlier reserves by detour*old_upper_load
// plus new_item_weight*new_distance_home. All accepted margins strictly
// positive, which certifies the partial trip can then finish safely.
template<int Capacity>
struct StackDurabilityReserve {
  struct Entry{long long margin=0,above=0;};
  std::array<Entry,Capacity> layer{};
  int size=0,home=0;

  bool append(long long weight,long long durability,int path,int next_home){
    assert(weight>=0&&durability>0&&path>=0&&next_home>=0);
    const long long delta=static_cast<long long>(path)+next_home-home;
    assert(delta>=0);
    if(size>=Capacity)return false;
    for(int i=0;i<size;++i)
      if(layer[i].margin<=layer[i].above*delta+weight*next_home)return false;
    for(int i=0;i<size;++i){
      layer[i].margin-=layer[i].above*delta+weight*next_home;
      layer[i].above+=weight;
    }
    layer[size++]={durability,0};
    home=next_home;
    return true;
  }
};
