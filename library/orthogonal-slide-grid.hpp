#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

// Sliding in any axis direction stops before the nearest wall or boundary.
// A single wall edit refreshes only its row and column, in O(h+w).
class OrthogonalSlideGrid {
    int h_,w_;
    std::vector<unsigned char> wall_;
    std::vector<std::array<int,4>> stop_; // left,right,up,down
    void row(int r) {
        int left=r*w_;
        for(int c=0;c<w_;++c) {
            int v=r*w_+c;
            if(wall_[v]) left=v+1,stop_[v][0]=v;
            else stop_[v][0]=left;
        }
        int right=(r+1)*w_-1;
        for(int c=w_-1;c>=0;--c) {
            int v=r*w_+c;
            if(wall_[v]) right=v-1,stop_[v][1]=v;
            else stop_[v][1]=right;
        }
    }
    void col(int c) {
        int up=c;
        for(int r=0;r<h_;++r) {
            int v=r*w_+c;
            if(wall_[v]) up=v+w_,stop_[v][2]=v;
            else stop_[v][2]=up;
        }
        int down=(h_-1)*w_+c;
        for(int r=h_-1;r>=0;--r) {
            int v=r*w_+c;
            if(wall_[v]) down=v-w_,stop_[v][3]=v;
            else stop_[v][3]=down;
        }
    }
public:
    OrthogonalSlideGrid(int h,int w,const std::vector<unsigned char>& wall)
        : h_(h),w_(w),wall_(wall),stop_(h*w) {
        assert(h>0 && w>0 && int(wall.size())==h*w);
        for(int r=0;r<h_;++r) row(r);
        for(int c=0;c<w_;++c) col(c);
    }
    bool blocked(int v) const {return wall_[v];}
    const std::array<int,4>& destinations(int v) const {return stop_[v];}
    void set_blocked(int v,bool blocked) {
        assert(0<=v && v<h_*w_);
        if(bool(wall_[v])==blocked) return;
        wall_[v]=blocked;
        row(v/w_);
        col(v%w_);
    }
    // p and next must be distinct; blocked cells carry no live mass.
    template<class Real>
    void redistribute(const std::vector<Real>& p,std::vector<Real>& next) const {
        assert(&p!=&next && p.size()==stop_.size() && next.size()==p.size());
        std::fill(next.begin(),next.end(),Real(0));
        for(int v=0;v<h_*w_;++v) if(!wall_[v] && p[v]!=Real(0)) {
            const Real q=Real(.25)*p[v];
            auto [a,b,c,d]=stop_[v];
            next[a]+=q;next[b]+=q;next[c]+=q;next[d]+=q;
        }
    }
};
