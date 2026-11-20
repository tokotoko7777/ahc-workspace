#pragma once
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>

// Rectangular occupancy up to 64 rows and columns. Query excludes the
// source cell (even when occupied); endpoints are flattened row-major.
class GridLineOccupancy64 {
    using Word=std::uint64_t;
    int height_,width_;
    std::array<Word,64> rows_{},cols_{};
    static int lower(Word mask,int i) {
        const Word before=mask&((Word{1}<<i)-1);
        return before ? std::bit_width(before)-1 : -1;
    }
    static int higher(Word mask,int i) {
        const Word after=(mask>>i)>>1;
        return after ? i+1+std::countr_zero(after) : -1;
    }
public:
    GridLineOccupancy64(int h,int w):height_(h),width_(w) {
        assert(0<h && h<=64 && 0<w && w<=64);
    }
    void set(int r,int c,bool occupied) {
        assert(0<=r && r<height_ && 0<=c && c<width_);
        const Word x=Word{1}<<c, y=Word{1}<<r;
        if(occupied) { rows_[r]|=x; cols_[c]|=y; }
        else { rows_[r]&=~x; cols_[c]&=~y; }
    }
    std::array<int,4> visible(int r,int c) const {
        assert(0<=r && r<height_ && 0<=c && c<width_);
        const int left=lower(rows_[r],c), right=higher(rows_[r],c);
        const int up=lower(cols_[c],r), down=higher(cols_[c],r);
        return {up<0?-1:up*width_+c,down<0?-1:down*width_+c,
                left<0?-1:r*width_+left,right<0?-1:r*width_+right};
    }
};
