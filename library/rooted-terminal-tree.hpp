#pragma once
#include <cassert>
#include <vector>

// 根付き木から、印の付いた頂点と根を結ぶ最小部分木を抽出する。
// order は親を子より先に含む全頂点順、parent[root]==root。
// needed は呼出側で用意する scratch。返ったあと needed[v] は親辺の要否。
template<class Range,class Required>
void rooted_terminal_tree(const Range& order,const Range& parent,Required required,
                          std::vector<unsigned char>& needed) {
  assert(order.size()==parent.size());
  needed.assign(parent.size(),0);
  for(auto it=order.rbegin();it!=order.rend();++it){
    int v=*it;
    if(needed[v]||required(v)){
      needed[v]=1;
      needed[parent[v]]=1;
    }
  }
}
