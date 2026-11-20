#pragma once
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>

// 64x64 以下の正方格子を、行・列・2対角方向の64bitマスクで管理。
// 方向は E,NE,N,NW,W,SW,S,SE。点集合と辺集合は別のインスタンスにする。
class SquareLatticeLineBits {
 public:
  using Mask = std::uint64_t;
  static constexpr std::array<int,8> dx{1,1,0,-1,-1,-1,0,1};
  static constexpr std::array<int,8> dy{0,1,1,1,0,-1,-1,-1};

  explicit SquareLatticeLineBits(int n) : n_(n) { assert(n>0 && n<=64); }
  bool inside(int x,int y) const { return 0<=x && x<n_ && 0<=y && y<n_; }

  bool marked(int x,int y) const {
    assert(inside(x,y));
    return (bits_[0][y]>>x)&1;
  }
  void mark(int x,int y) {
    assert(inside(x,y));
    for(int f=0;f<4;++f)
      bits_[f][line(x,y,f)] |= Mask(1)<<position(x,y,f);
  }
  // 自分の点は候補に含めない。点がないときは0。
  int nearest_steps(int x,int y,int direction) const {
    const int f=direction%4, p=position(x,y,f);
    Mask mask=bits_[f][line(x,y,f)];
    if(direction<4) {
      mask &= ~below(p+1);
      return mask?std::countr_zero(mask)-p:0;
    }
    mask &= below(p);
    return mask?p-(63-std::countl_zero(mask)):0;
  }
  // 両端を除いた、長さ1単位の格子点が存在するか。
  bool inner_mark(int x,int y,int direction,int length) const {
    auto [f,p,q]=span(x,y,direction,length);
    return bits_[f][line(x,y,f)] & (below(q)-below(p+1));
  }
  // 単位辺 [i,i+1] を低い側 i のbitsに記録。端点だけ共有する辺は別。
  bool overlaps_segment(int x,int y,int direction,int length) const {
    auto [f,p,q]=span(x,y,direction,length);
    return bits_[f][line(x,y,f)] & (below(q)-below(p));
  }
  void add_segment(int x,int y,int direction,int length) {
    auto [f,p,q]=span(x,y,direction,length);
    bits_[f][line(x,y,f)] |= below(q)-below(p);
  }

 private:
  int n_;
  std::array<std::array<Mask,127>,4> bits_{};
  static Mask below(int p) { return p==64?~Mask(0):(Mask(1)<<p)-1; }
  static int position(int x,int y,int f) { return f<2?x:y; }
  int line(int x,int y,int f) const {
    if(f==0) return y;
    if(f==1) return x-y+n_-1;
    if(f==2) return x;
    return x+y;
  }
  std::array<int,3> span(int x,int y,int d,int length) const {
    assert(length>0 && inside(x,y) && inside(x+dx[d]*length,y+dy[d]*length));
    const int f=d%4,p=position(x,y,f);
    return d<4?std::array{f,p,p+length}:std::array{f,p-length,p};
  }
};
