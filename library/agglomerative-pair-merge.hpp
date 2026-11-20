#pragma once
#include <cassert>
#include <cstddef>
#include <functional>
#include <queue>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

template<class Node>
struct PairMergeTree {
  std::vector<Node> values;
  std::vector<std::pair<int,int>> children; // Leaves use {-1,-1}.
};

// Greedily join maximum-ranked live pairs. Rank depends only on the two
// fixed node values, never on external mutable state. Equal ranks prefer
// the pair with the larger lexicographic (older id, newer id).
// n-1 merges, O(n^2 log n) time and O(n^2) memory.
template<class Node,class Rank,class Merge>
PairMergeTree<Node> agglomerative_pair_merge(std::vector<Node> initial,Rank rank,Merge merge){
  using Key=std::decay_t<decltype(rank(std::declval<const Node&>(),std::declval<const Node&>()))>;
  using Link=std::tuple<Key,int,int>;
  const int n=int(initial.size());
  PairMergeTree<Node> result{std::move(initial),std::vector<std::pair<int,int>>(n,{-1,-1})};
  if(n<=1)return result;
  result.values.reserve(2*n-1);
  result.children.reserve(2*n-1);
  std::vector<char> active(2*n-1,true);
  std::vector<Link> candidates;
  candidates.reserve(std::size_t(n)*(n-1)/2);
  for(int a=0;a<n;++a)for(int b=a+1;b<n;++b)
    candidates.emplace_back(rank(result.values[a],result.values[b]),a,b);
  std::priority_queue<Link> heap(std::less<Link>(),std::move(candidates));
  for(int next=n;next<2*n-1;++next){
    while(!heap.empty()&&(!active[std::get<1>(heap.top())]||!active[std::get<2>(heap.top())]))
      heap.pop();
    assert(!heap.empty());
    auto [key,a,b]=heap.top();
    heap.pop();
    active[a]=active[b]=false;
    result.values.push_back(merge(result.values[a],result.values[b]));
    result.children.push_back({a,b});
    if(next+1<2*n-1)for(int i=0;i<next;++i)if(active[i])
      heap.emplace(rank(result.values[i],result.values[next]),i,next);
  }
  return result;
}
