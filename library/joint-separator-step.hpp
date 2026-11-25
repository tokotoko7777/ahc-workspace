#pragma once
#include <array>
#include <bitset>
#include <cstddef>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace ahc {
struct JointSeparatorStep { int area=0, assignments=0, floods=0; std::string commands; };
// Complete simultaneous action neighborhood for up to six workers.
// Moves may overlap and use pre-existing open edges; walls and moves cannot
// share a destination. The caller supplies legal construction cells before
// movement, excluding all current worker cells and condition-specific vetoes.
// Adjacent(vertex) must return four neighbors in consistent caller-defined order;
// use -1 for absent directions. Lowercase abcd builds, uppercase ABCD moves.
// Unlike a subset check, this searches all legal build and one-edge move choices.
// If infeasible, area==0 and commands is empty.

template<std::size_t Size>
inline JointSeparatorStep joint_separator_step(const std::bitset<Size>& walls,const std::bitset<Size>& legal,
                   const std::bitset<Size>& excluded,
                    std::span<const int> humans, const auto& adjacent) {
  using Mask=std::bitset<Size>;
  JointSeparatorStep best;
  const int m=humans.size();
  if(m<1||m>6)return best;
  std::array<int,6> worker_cell;
  worker_cell.fill(-1);
  Mask blocked=walls;
  constexpr char Move[]="ABCD";
  constexpr char Build[]="abcd";
  auto examine=[&](){
    ++best.assignments;
    Mask tried;
    std::array<int,5> roots;
    roots[0]=humans[0];int nroot=1;
    if(worker_cell[0]<0)for(int q:adjacent(humans[0]))if(q>=0&&!blocked[q])roots[nroot++]=q;
    for(int r=0;r<nroot;++r){
      const int root=roots[r];
      if(tried[root])continue;
      Mask seen;
      std::array<int,Size> q;int len=1;q[0]=root;seen[root]=true;
      bool clear=true;
      ++best.floods;
      for(int i=0;i<len;++i){
        const int p=q[i];
        if(excluded[p]){clear=false;break;}
        for(int v:adjacent(p))if(v>=0&&!blocked[v]&&!seen[v]){seen[v]=true;q[len++]=v;}
      }
      tried|=seen;
      if(!clear||len<=best.area)continue;
      std::string cmd(m,'.');
      bool possible=true;
      for(int i=0;i<m;++i){
        const int p=humans[i];
        if(worker_cell[i]>=0){
          if(!seen[p]){possible=false;break;}
          auto near=adjacent(p);
          for(int d=0;d<4;++d)if(near[d]==worker_cell[i]){cmd[i]=Build[d];break;}
        }else if(!seen[p]){
          auto near=adjacent(p);
          for(int d=0;d<4;++d)if(near[d]>=0&&seen[near[d]]){cmd[i]=Move[d];break;}
          if(cmd[i]=='.'){possible=false;break;}
        }
      }
      if(possible){best.area=len;best.commands=std::move(cmd);}
    }
  };
  auto dfs=[&](auto&& f,int person)->void{
    if(person==m){examine();return;}
    f(f,person+1);
    for(int v:adjacent(humans[person])){
      if(v<0||!legal[v]||blocked[v])continue;
      worker_cell[person]=v;blocked.set(v);
      f(f,person+1);
      blocked.reset(v);
    }
    worker_cell[person]=-1;
  };
  dfs(dfs,0);
  return best;
}
}
