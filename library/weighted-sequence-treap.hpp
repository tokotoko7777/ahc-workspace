#ifndef AHC_WEIGHTED_SEQUENCE_TREAP_HPP
#define AHC_WEIGHTED_SEQUENCE_TREAP_HPP
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

// Implicit treap over a permutation of stable item IDs. Augments a sequence
// with sum of weights and the sum of index*weight. All quantities including
// intermediate products must fit int64_t. Reverse, index lookup, and item
// rank take expected O(log n); no allocations after construction.
class WeightedSequenceTreap {
  struct Node{
    int child[2]{-1,-1},parent=-1,count=1;
    std::uint32_t priority=0;
    bool reversed=false;
    std::int64_t weight=0,sum=0,moment=0;
  };
  std::vector<Node> nodes;
  std::vector<int> ancestors;
  int root=-1;
  int count(int t)const{return t<0?0:nodes[t].count;}
  std::int64_t sum(int t)const{return t<0?0:nodes[t].sum;}
  std::int64_t moment(int t)const{return t<0?0:nodes[t].moment;}
  void attach(int t,int i,int child){
    nodes[t].child[i]=child;
    if(child>=0)nodes[child].parent=t;
  }
  void pull(int t){
    auto& x=nodes[t];
    const int l=x.child[0],r=x.child[1],k=count(l);
    x.count=k+1+count(r);x.sum=sum(l)+x.weight+sum(r);
    x.moment=moment(l)+k*x.weight+moment(r)+(k+1)*sum(r);
  }
  void reverse_subtree(int t){
    if(t<0)return;
    auto& x=nodes[t];x.reversed=!x.reversed;
    std::swap(x.child[0],x.child[1]);
    x.moment=(x.count-1)*x.sum-x.moment;
  }
  void push(int t){
    auto& x=nodes[t];
    if(x.reversed){
      for(int u:x.child)reverse_subtree(u);
      x.reversed=false;
    }
  }
  int join(int a,int b){
    if(a<0||b<0){
      int z=std::max(a,b);
      if(z>=0)nodes[z].parent=-1;
      return z;
    }
    if(nodes[a].priority>nodes[b].priority){
      push(a);attach(a,1,join(nodes[a].child[1],b));pull(a);
      nodes[a].parent=-1;return a;
    }
    push(b);attach(b,0,join(a,nodes[b].child[0]));pull(b);
    nodes[b].parent=-1;return b;
  }
  void split(int t,int k,int& l,int& r){
    if(t<0){l=r=-1;return;}
    if(k<=0){l=-1;r=t;nodes[t].parent=-1;return;}
    if(k>=count(t)){l=t;r=-1;nodes[t].parent=-1;return;}
    push(t);
    const int m=count(nodes[t].child[0]);
    if(k<=m){
      int right;
      split(nodes[t].child[0],k,l,right);attach(t,0,right);
      r=t;
    }else{
      int left;
      split(nodes[t].child[1],k-m-1,left,r);attach(t,1,left);
      l=t;
    }
    pull(t);nodes[t].parent=-1;
  }
public:
  template<class Weights>
  WeightedSequenceTreap(const std::vector<int>& order,const Weights& weights,
                        std::uint32_t seed=9013):nodes(order.size()){
    assert(order.size()==weights.size()&&seed!=0);
    ancestors.reserve(order.size());
    for(int id:order){
      assert(0<=id&&id<int(nodes.size())&&nodes[id].priority==0);
      seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
      auto& a=nodes[id];a.weight=a.sum=weights[id];a.priority=seed;
      root=join(root,id);
    }
  }
  int size()const{return count(root);}
  std::int64_t indexed_sum()const{return moment(root);}
  int at(int k){
    assert(0<=k&&k<size());
    int t=root;
    while(true){
      push(t);auto& x=nodes[t];
      const int left=count(x.child[0]);
      if(k==left)return t;
      if(k<left)t=x.child[0];
      else{k-=left+1;t=x.child[1];}
    }
  }
  int rank(int id){
    assert(0<=id&&id<int(nodes.size()));
    ancestors.clear();
    for(int t=id;t>=0;t=nodes[t].parent)ancestors.push_back(t);
    for(auto p=ancestors.rbegin();p!=ancestors.rend();++p)push(*p);
    int k=count(nodes[id].child[0]);
    for(int t=id,p=nodes[t].parent;p>=0;t=p,p=nodes[t].parent)
      if(nodes[p].child[1]==t)k+=count(nodes[p].child[0])+1;
    return k;
  }
  template<class Accept>
  bool try_reverse(int l,int r,Accept accept){
    assert(0<=l&&l<=r&&r<=size());
    if(r-l<=1)return false;
    int a,b,c;split(root,l,a,b);split(b,r-l,b,c);
    const auto delta=(count(b)-1)*sum(b)-2*moment(b);
    const bool yes=accept(delta);
    if(yes)reverse_subtree(b);
    root=join(a,join(b,c));
    return yes;
  }
  std::vector<int> order(){
    std::vector<int> result;
    result.reserve(size());
    auto dfs=[&](auto&& self,int t)->void{
      if(t<0)return;
      push(t);
      self(self,nodes[t].child[0]);result.push_back(t);self(self,nodes[t].child[1]);
    };
    dfs(dfs,root);return result;
  }
};
#endif
