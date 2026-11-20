#pragma once
// Include after Board and Geometry. Exact cycle scoring on one 2x2 patch.
class ContractedPatch {
 public:
  using Rot = std::array<int,4>;
  std::array<int,4> cells;

  ContractedPatch(Board& board, int top_left)
      : cells{top_left,top_left+1,top_left+W,top_left+W+1} {
    assert(top_left/W + 1 < H && top_left%W + 1 < W);
    for (int u=0;u<4;++u) {
      int v=cells[u], t=board.input[v];
      period_[u]=board.period[v];
      for (int r=0;r<period_[u];++r,t=rotate_tile[t]) states_[u][r]=t;
      for (int d=0;d<4;++d) {
        int nxt=board.adj[v][d];
        const int inner=local(nxt);
        if (inner>=0) {
          end_[u][d]={inner,d^2,0};
          continue;
        }
        int len=0, incoming=d^2;
        while(nxt>=0){
          int x=local(nxt);
          if(x>=0){
            end_[u][d]={x,incoming,len};
            break;
          }
          int out=to[board.orientation[nxt]][incoming];
          if(out<0) break;
          ++len;
          assert(len<=2*N);
          nxt=board.adj[nxt][out];
          incoming=out^2;
        }
      }
    }
    board.scanner.scan(
        [&](int v){return local(v/2)<0 && v%2<geo.size[board.orientation[v/2]];},
        [&](int v){
          auto a=board.edges[v];
          for(int& u:a) if(u>=0 && local(u/2)>=0) u=-1;
          return a;
        },
        [&](int len,int a,int){if(a<0) keep(outside_,len);});
  }

  std::array<int,2> longest(Rot r) const {
    const std::array<int,4> types{states_[0][r[0]],states_[1][r[1]],
                                  states_[2][r[2]],states_[3][r[3]]};
    std::array<std::array<Link,2>,8> links{};
    unsigned active=0;
    for (int u=0;u<4;++u) {
      int t=types[u];
      for(int j=0;j<geo.size[t];++j) {
        const int vertex=2*u+j;
        active|=1u<<vertex;
        for(int k=0;k<2;++k) {
          auto stop=end_[u][geo.endpoints[t][j][k]];
          if(stop.tile<0) continue;
          int other=geo.segment[types[stop.tile]][stop.side];
          if(other<0) continue;
          links[vertex][k]={2*stop.tile+other,stop.length};
        }
      }
    }
    auto result=outside_;
    while(active){
      unsigned pending=active & (~active+1u);
      active^=pending;
      int size=0, weight=0;
      bool closed=true;
      while(pending){
        int v=std::countr_zero(pending);
        pending^=1u<<v;
        ++size;
        for(auto e:links[v]){
          if(e.to<0){
            closed=false;
          }else{
            weight+=e.length;
            unsigned bit=1u<<e.to;
            if(active&bit) pending|=bit,active^=bit;
          }
        }
      }
      if(closed){
        assert(weight%2==0);
        keep(result,size+weight/2);
      }
    }
    return result;
  }

  template<class F>
  void enumerate(F f) const {
    for(int a=0;a<period_[0];++a) for(int b=0;b<period_[1];++b)
      for(int c=0;c<period_[2];++c) for(int d=0;d<period_[3];++d) {
        Rot r{a,b,c,d};
        f(r,longest(r));
      }
  }

 private:
  struct End {int tile=-1,side=-1,length=0;};
  struct Link {int to=-1,length=0;};
  std::array<std::array<End,4>,4> end_{};
  std::array<std::array<int,4>,4> states_{};
  std::array<int,4> period_{};
  std::array<int,2> outside_{};
  int local(int v) const {
    for(int i=0;i<4;++i) if(cells[i]==v) return i;
    return -1;
  }
  static void keep(std::array<int,2>& a,int length){
    if(length>=a[0]) a[1]=a[0],a[0]=length;
    else if(length>a[1]) a[1]=length;
  }
};

struct PatchComparison {
  std::array<int,N> single, joint;
  int single_length, joint_length, patches=0;
};

inline PatchComparison compare_patches(Board& b,const std::array<int,N>& start,
                                       int length,Clock::time_point stop) {
  for(int v=0;v<N;++v) b.store(v,start[v]);
  for(int v=0;v<N;++v) b.build_links(v);
  PatchComparison ans{start,start,length,length};
  constexpr int count=(H-1)*(W-1);
  for(int i=0;i<count;++i) {
    if(Clock::now()>=stop) break;
    const int pick=(i*251+19)%count; // coprime deterministic sweep across all patches
    const int v=pick/(W-1)*W + pick%(W-1);
    ContractedPatch patch(b,v);
    patch.enumerate([&](const auto& r,const auto& cycles){
      int change=0;
      for(int k=0;k<4;++k) change+=r[k]!=start[patch.cells[k]];
      if(change==1 && cycles[0]>ans.single_length){
        ans.single=start;
        ans.single_length=cycles[0];
        for(int k=0;k<4;++k) ans.single[patch.cells[k]]=r[k];
      }
      if(cycles[0]>ans.joint_length){
        ans.joint=start;
        ans.joint_length=cycles[0];
        for(int k=0;k<4;++k) ans.joint[patch.cells[k]]=r[k];
      }
    });
    ++ans.patches;
  }
  return ans;
}
