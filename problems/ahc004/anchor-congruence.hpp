#pragma once
#include "observed-cycles.hpp"
#include <array>
#include <bit>
#include <queue>
#include <tuple>

namespace ahc004 {
struct AnchorLayout {
  std::array<int,3> id{}, lane{}, phase{};
  int matched=0, eligible=0, distinct=0;
  std::array<uint64_t,20> domains{};
};
struct AnchorSearch {
  std::vector<AnchorLayout> layouts;
  int examined=0, exact_checked=0;
  bool capped=false;
};

template<bool PreferNative=false>
inline int match_columns(const std::array<uint64_t,20>& domains,
                         std::array<int,20>* assignment=nullptr, uint64_t native=0) {
  if constexpr(PreferNative) {
    // Greedy augmentations on the donor side preserve all donors already
    // matched. Taking native donors first therefore attains maximum native
    // count among maximum-cardinality assignments.
    uint32_t lanes[64]{};
    for(int col=0;col<20;++col) for(uint64_t bits=domains[col];bits;bits&=bits-1)
      lanes[std::countr_zero(bits)]|=uint32_t(1)<<col;
    std::array<int,20> col_mate;
    col_mate.fill(-1);
    int size=0;
    for(int pass=0;pass<2;++pass) for(int id=0;id<64;++id){
      if(int((native>>id)&1)!=1-pass || !lanes[id])continue;
      uint32_t seen=0;
      const auto augment=[&](const auto& self,int cur)->bool{
        while(const uint32_t bits=lanes[cur]&~seen){
          const int col=std::countr_zero(bits);
          seen|=uint32_t(1)<<col;
          if(col_mate[col]<0 || self(self,col_mate[col])){
            col_mate[col]=cur;
            return true;
          }
        }
        return false;
      };
      size+=augment(augment,id);
    }
    if(assignment)*assignment=col_mate;
    return size;
  }
  std::array<int,64> mate;
  mate.fill(-1);
  int result=0;
  for (int lane=0; lane<20; ++lane) {
    uint64_t seen=0;
    const auto dfs=[&](const auto& self,int col)->bool {
      while (const uint64_t bits=domains[col]&~seen) {
        const int id=std::countr_zero(bits);
        seen|=uint64_t(1)<<id;
        if (mate[id]<0 || self(self,mate[id])) {
          mate[id]=col;
          return true;
        }
      }
      return false;
    };
    result+=dfs(dfs,lane);
  }
  if (assignment) {
    assignment->fill(-1);
    for (int id=0;id<64;++id) if (mate[id]>=0) (*assignment)[mate[id]]=id;
  }
  return result;
}

// Search full closed-line hypotheses only. Returned rows and the set of
// eligible (column lane, cycle ID) edges are exact, not proof of global layout.
inline AnchorSearch three_parallel_anchors(
    const std::vector<Cycle>& bank, int visit_cap=1368000,
    const std::function<bool()>& deadline=[] { return false; },
    int shortlist_cap=128, std::array<int,5> anchor_ids={0,1,2,3,4}) {
  AnchorSearch out;
  const int n=std::min<int>(64,bank.size());
  if (n<24 || shortlist_cap<=0 || visit_cap<=0) return out;
  auto sorted_ids=anchor_ids;
  std::sort(sorted_ids.begin(),sorted_ids.end());
  if (sorted_ids.front()<0 || sorted_ids.back()>=n ||
      std::adjacent_find(sorted_ids.begin(),sorted_ids.end())!=sorted_ids.end()) return out;
  const auto quality=[](const AnchorLayout& t) {
    return std::tuple{t.matched,t.eligible,t.distinct};
  };
  const auto less=[&](const AnchorLayout& a,const AnchorLayout& b) {
    return quality(a)>quality(b);
  };
  std::priority_queue<AnchorLayout,std::vector<AnchorLayout>,decltype(less)> shortlist(less);
  int digits[64][40]{};
  for(int id=0;id<n;++id)
    for(int t=0;t<40;++t) digits[id][t]=bank[id].letters[t%20]-'A';

  // Interleave identity triples, so time caps cannot spend all on an early
  // wrong-orientation triple. Reflections are NOT allowed.
  for(int r1=1;r1<20;++r1) for(int r2=r1+1;r2<20;++r2) {
    std::array<uint64_t,512> signatures{};
    for(int id=0;id<n;++id) for(int p=0;p<20;++p)
      signatures[(digits[id][p]<<6)|(digits[id][p+r1]<<3)|digits[id][p+r2]] |= uint64_t(1)<<id;
    for(int aa=0;aa<5;++aa) for(int bb=aa+1;bb<5;++bb) for(int cc=bb+1;cc<5;++cc)
    for(int swap_lanes=0;swap_lanes<2;++swap_lanes) {
      const int a=anchor_ids[aa], b=anchor_ids[bb], c=anchor_ids[cc];
      const int x=swap_lanes?r2:r1, y=swap_lanes?r1:r2;
      const int j=swap_lanes?c:b, k=swap_lanes?b:c;
      const uint64_t barred=(uint64_t(1)<<a)|(uint64_t(1)<<b)|(uint64_t(1)<<c);
      for(int p=0;p<20;++p) for(int q=0;q<20;++q) {
        if (out.examined==visit_cap || ((out.examined&1023)==0 && deadline())) {
          out.capped=true;
          goto done;
        }
        ++out.examined;
        const int min_match=shortlist.size()==size_t(shortlist_cap)
            ? std::max(n-22,shortlist.top().matched) : n-22;
        if (min_match>20) continue;
        AnchorLayout candidate;
        uint64_t all=0;
        for(int t=0;t<20;++t) {
          const auto mask=signatures[(digits[a][t]<<6)|(digits[j][t+(swap_lanes?q:p)]<<3)|
                                    digits[k][t+(swap_lanes?p:q)]] & ~barred;
          candidate.domains[t]=mask;
          candidate.eligible += mask!=0;
          all|=mask;
          if(candidate.eligible+19-t<min_match) break;
        }
        if(candidate.eligible<min_match) continue;
        candidate.distinct=std::popcount(all);
        if(candidate.distinct<min_match) continue;
        if(shortlist.size()==size_t(shortlist_cap) &&
           std::min(candidate.distinct,candidate.eligible)==shortlist.top().matched &&
           std::pair{candidate.eligible,candidate.distinct} <=
               std::pair{shortlist.top().eligible,shortlist.top().distinct}) continue;
        ++out.exact_checked;
        candidate.matched=match_columns(candidate.domains);
        if(candidate.matched<min_match) continue;
        candidate.id={a,b,c};
        candidate.lane={0,x,y};
        candidate.phase={0,p,q};
        if(shortlist.size()==size_t(shortlist_cap)) {
          if(quality(candidate)<=quality(shortlist.top())) continue;
          shortlist.pop();
        }
        shortlist.push(candidate);
      }
    }
  }
done:
  while(!shortlist.empty()) {
    out.layouts.push_back(shortlist.top());
    shortlist.pop();
  }
  std::reverse(out.layouts.begin(),out.layouts.end());
  return out;
}
struct FourthRow {
  int parent=-1, id=-1, lane=-1, phase=-1, matched=0, eligible=0;
};
struct FourthSearch {
  std::vector<FourthRow> ranked;
  int examined=0;
  bool capped=false;
};

// Recheck shortlisted triples against a fourth exact row. Keep one strongest
// extension per triple; do not mistake a high crossing count for identity.
inline FourthSearch fourth_row_extensions(
    const std::vector<Cycle>& bank, const std::vector<AnchorLayout>& layouts,
    int budget=2000000, const std::function<bool()>& deadline=[] { return false; },
    bool packed_input=false) {
  FourthSearch out;
  const int n=std::min<int>(64,bank.size());
  unsigned char digit[64][40]{};
  if (packed_input) for (int id=0; id<n; ++id) for(int p=0;p<40;++p)
    digit[id][p]=bank[id].letters[p%20]-'A';
  for(int at=0; at<int(layouts.size()); ++at) {
    if(deadline()) {out.capped=true;break;}
    const auto& t=layouts[at];
    uint64_t domains[20][20][8]{};
    const auto used=[&](int id) {return id==t.id[0]||id==t.id[1]||id==t.id[2];};
    const auto row_used=[&](int r) {return r==0||r==t.lane[1]||r==t.lane[2];};
    for(int col=0;col<20;++col) {
      uint64_t bits=t.domains[col];
      while (bits) {
        const int id=std::countr_zero(bits);
        bits&=bits-1;
        for(int phase=0;phase<20;++phase) {
          bool match=true;
          for(int j=0;j<3;++j)
            if(packed_input ? digit[id][phase+t.lane[j]] != digit[t.id[j]][col+t.phase[j]]
                            : bank[id].letters[(phase+t.lane[j])%20] !=
                              bank[t.id[j]].letters[(col+t.phase[j])%20]) {match=false;break;}
          if(match) for(int r=0;r<20;++r)
            domains[r][col][packed_input ? digit[id][phase+r] : bank[id].letters[(phase+r)%20]-'A']|=uint64_t(1)<<id;
        }
      }
    }
    FourthRow best;
    for(int r=1;r<20;++r) if(!row_used(r))
    for(int id=0;id<n;++id) if(!used(id)) for(int phase=0;phase<20;++phase) {
      if(out.examined>=budget||((out.examined&1023)==0&&deadline())) {
        out.capped=true;
        goto done4;
      }
      ++out.examined;
      std::array<uint64_t,20> mask{};
      int count=0;
      const int needed=std::max(n-23,best.matched);
      for(int col=0;col<20;++col) {
        mask[col]=domains[r][col][packed_input ? digit[id][phase+col] : bank[id].letters[(phase+col)%20]-'A'] & ~(uint64_t(1)<<id);
        count+=mask[col]!=0;
        if(count+19-col<needed) break;
      }
      if(count<needed) continue;
      const int match=match_columns(mask);
      if(match<needed) continue;
      if(best.id<0||std::pair{match,count}>std::pair{best.matched,best.eligible})
        best={at,id,r,phase,match,count};
    }
    if(best.id>=0) out.ranked.push_back(best);
  }
done4:
  std::stable_sort(out.ranked.begin(),out.ranked.end(),[](const FourthRow& a,const FourthRow& b) {
    return std::pair{a.matched,a.eligible}>std::pair{b.matched,b.eligible};
  });
  return out;
}
// Equality under global shifts and transposition, which preserve the
// problem's two forward directions (reversal is not a symmetry).
inline std::string geometry_shape_key(const std::vector<std::string>& board) {
  constexpr int N=20;
  if (board.size()!=N) return {};
  char best[N*N];
  bool have=false;
  for(int transpose=0;transpose<2;++transpose) {
    char doubled[N][2*N];
    for(int y=0;y<N;++y)for(int x=0;x<N;++x)
      doubled[y][x]=doubled[y][x+N]=transpose?board[x][y]:board[y][x];
    for(int y=0;y<N;++y)for(int x=0;x<N;++x) {
      int cmp=have?0:-1;
      for(int i=0;i<N && cmp==0;++i)
        cmp=std::char_traits<char>::compare(doubled[(y+i)%N]+x,best+i*N,N);
      if(cmp<0) {
        for(int i=0;i<N;++i)
          std::char_traits<char>::copy(best+i*N,doubled[(y+i)%N]+x,N);
        have=true;
      }
    }
  }
  return {best,N*N};
}
struct RealizedAnchor {
  std::vector<std::string> board;
  int rows=0, columns=0;
};

// Recover a phase witness for every selected column edge. Then extend the
// disjoint line inventory on either axis, only through exact intersections.
template<bool PreferNative=false>
inline RealizedAnchor realize_anchor(const std::vector<Cycle>& bank,
                                     const AnchorLayout& anchor,
                                     const FourthRow& fourth,
                                     const std::function<bool()>& deadline=[] {return false;},
                                     uint64_t native=0) {
  constexpr int N=20;
  const int k=std::min<int>(bank.size(),64);
  RealizedAnchor out{std::vector<std::string>(N,std::string(N,'.')),0,0};
  auto& board=out.board;
  std::array<std::array<bool,N>,2> lanes{};
  std::array<bool,64> taken{};
  const auto compatible=[&](int id,int axis,int lane,int phase) {
    for(int j=0;j<N;++j) {
      char old=axis==0?board[lane][j]:board[j][lane];
      if(old!='.' && old!=bank[id].letters[(phase+j)%N]) return false;
    }
    return true;
  };
  const auto place=[&](int id,int axis,int lane,int phase) {
    if(taken[id] || lanes[axis][lane] || !compatible(id,axis,lane,phase)) return false;
    for(int j=0;j<N;++j)
      (axis==0?board[lane][j]:board[j][lane])=bank[id].letters[(phase+j)%N];
    taken[id]=lanes[axis][lane]=true;
    if(axis==0) ++out.rows; else ++out.columns;
    return true;
  };
  for(int i=0;i<3;++i) if(!place(anchor.id[i],0,anchor.lane[i],anchor.phase[i])) return {};
  if(!place(fourth.id,0,fourth.lane,fourth.phase)) return {};
  std::array<uint64_t,N> masks{};
  std::array<std::array<int,64>,N> phases{};
  for(int lane=0;lane<N;++lane) for(int id=0;id<k;++id) {
    if(taken[id] || !(anchor.domains[lane]>>id&1)) continue;
    for(int phase=0;phase<N;++phase) if(compatible(id,1,lane,phase)) {
      masks[lane]|=uint64_t(1)<<id;
      phases[lane][id]=phase;
      break;
    }
  }
  std::array<int,N> assignment;
  if(match_columns<PreferNative>(masks,&assignment,native)!=fourth.matched) return {};
  for(int lane=0;lane<N;++lane) if(assignment[lane]>=0)
    if(!place(assignment[lane],1,lane,phases[lane][assignment[lane]])) return {};
  const auto extend=[&](int id) {
    if(taken[id])return;
    for(int axis=0;axis<2 && !taken[id];++axis)
    for(int lane=0;lane<N && !taken[id];++lane) {
      if(lanes[axis][lane]) continue;
      for(int phase=0;phase<N;++phase) if(place(id,axis,lane,phase)) break;
    }
  };
  if constexpr(PreferNative) {
    for(int group=0;group<2;++group)
      for(int id=0;id<k && !deadline();++id) {
        if(int((native>>id)&1)==1-group)extend(id);
      }
  } else {
    for(int id=0;id<k && !deadline();++id)extend(id);
  }
  return out;
}
}  // namespace ahc004
