#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include "weighted-dsu.hpp"

// Greedily accumulate relative start constraints on equal-length circles.
// '.' is an unknown symbol. A conflicting join changes no component or pattern.
class CircularPatternComponents {
    int n_;
    WeightedDsu<int> dsu_;
    std::vector<std::string> model_;
    int normalize(int x) const {int r=x%n_;return r<0?r+n_:r;}
public:
    CircularPatternComponents(int period,const std::vector<std::string>& patterns)
        :n_(period),dsu_(static_cast<int>(patterns.size())),model_(patterns) {
        if(period<=0) throw std::invalid_argument("period must be positive");
        for(auto& p:model_) {
            if(p.size()>static_cast<std::size_t>(period)) throw std::invalid_argument("pattern exceeds period");
            p.resize(period,'.');
        }
    }
    bool join(int a,int b,int b_start_minus_a) {
        const int x=dsu_.leader(a),y=dsu_.leader(b);
        const int d=normalize(b_start_minus_a);
        if(x==y) return normalize(dsu_.difference(a,b))==d;
        const int shift=normalize(dsu_.potential(a)+d-dsu_.potential(b));
        for(int i=0;i<n_;++i) {
            char c=model_[y][i],p=model_[x][(i+shift)%n_];
            if(c!='.'&&p!='.'&&p!=c)return false;
        }
        dsu_.unite(a,b,d);
        if(dsu_.leader(a)==x) {
            for(int i=0;i<n_;++i)if(model_[y][i]!='.')model_[x][(i+shift)%n_]=model_[y][i];
        } else {
            for(int i=0;i<n_;++i)if(model_[x][i]!='.')model_[y][(i-shift+n_)%n_]=model_[x][i];
        }
        return true;
    }
    int leader(int i) {return dsu_.leader(i);}
    int offset(int i) {return normalize(dsu_.potential(i));}
    const std::string& pattern(int i) {return model_[dsu_.leader(i)];}
};
