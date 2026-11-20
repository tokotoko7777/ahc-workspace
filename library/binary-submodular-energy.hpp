#pragma once
#include <stdexcept>
#include "max-flow.hpp"

// Integer energy minimization over bool variables. x_i=1 denotes the
// source side of the minimum cut. Pair tables b00,b01,b10,b11 must satisfy
// b00+b11<=b01+b10. Finite capacities and their sum must fit in Capacity.
template<class Capacity>
class BinarySubmodularEnergy {
    int count_;
    MaxFlow<Capacity> flow_;
    Capacity constant_ = 0;
    bool solved_ = false;
    int source() const {return count_;}
    int sink() const {return count_+1;}
public:
    explicit BinarySubmodularEnergy(int count):count_(count),flow_(count+2){}
    void unary(int i,Capacity zero,Capacity one) {
        constant_+=std::min(zero,one);
        if(zero<one) flow_.add_edge(i,sink(),one-zero);
        if(one<zero) flow_.add_edge(source(),i,zero-one);
    }
    void pair(int a,int b,Capacity b00,Capacity b01,Capacity b10,Capacity b11) {
        const Capacity cross=b10+b01-b00-b11;
        if(cross<0) throw std::invalid_argument("pair energy is not submodular");
        constant_+=b00;
        unary(a,0,b11-b01);
        unary(b,0,b01-b00);
        if(cross) flow_.add_edge(a,b,cross);
    }
    void imply(int a,int b,Capacity violation_cost) {
        flow_.add_edge(a,b,violation_cost);
    }
    std::pair<Capacity,std::vector<char>> solve() {
        if(solved_) throw std::logic_error("solve can be called once");
        solved_=true;
        const Capacity value=constant_+flow_.flow(source(),sink());
        auto cut=flow_.min_cut(source());cut.resize(count_);
        return {value,std::move(cut)};
    }
};
