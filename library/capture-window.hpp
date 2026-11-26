#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

namespace ahc {
// Probability of visiting a marked state at any decision boundary in
// [first,last], inclusive. 'micro_steps' transitions per decision. Rows
// contain equiprobable destinations (duplicates represent weights);
// an empty row means the state remains unchanged. Board/graph must be
// fixed for the entire forecast. Scratch is reusable across calls.
// Only nonzero support is visited; probabilities use floating point.
class CaptureWindow {
    struct Mass {
        std::vector<double> weights;
        std::vector<int> support;
        explicit Mass(std::size_t n=0):weights(n) {support.reserve(n);}
        void clear() {for(int x:support)weights[x]=0.;support.clear();}
        void add(int x,double q) {
            if(q==0.) return;
            if(weights[x]==0.)support.push_back(x);
            weights[x]+=q;
        }
    };
    Mass buffers[2];
public:
    explicit CaptureWindow(std::size_t n):buffers{Mass(n),Mass(n)} {}
    template<class Link, class Accept>
    double probability(int start,int micro_steps,int first,int last,
                       const Link& links,const Accept& safe) {
        assert(start>=0 && static_cast<std::size_t>(start)<buffers[0].weights.size());
        assert(0<=first&&first<=last&&micro_steps>=0);
        auto* cur=&buffers[0];
        auto* next=&buffers[1];
        cur->clear();next->clear();
        cur->add(start,1.);
        double caught=0.;
        for(int t=0;t<=last;++t){
            if(t>=first){
                auto& support=cur->support;
                support.erase(std::remove_if(support.begin(),support.end(),[&](int x){
                    if(!safe(x))return false;
                    caught+=cur->weights[x];cur->weights[x]=0.;return true;
                }),support.end());
            }
            if(t==last || cur->support.empty())break;
            for(int k=0;k<micro_steps;++k){
                next->clear();
                for(int x:cur->support){
                    const auto& row=links(x);
                    if(row.empty()){next->add(x,cur->weights[x]);continue;}
                    const double each=cur->weights[x]/row.size();
                    for(int y:row)next->add(y,each);
                }
                std::swap(cur,next);
            }
        }
        return std::clamp(caught,0.,1.);
    }
};
}
