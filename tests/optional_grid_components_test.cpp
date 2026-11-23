#include "optional-grid-components.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <numeric>
#include <random>
#include <utility>

using namespace ahc;
using Solver = optional_grid_components<2>;
using Summary = Solver::summary_type;
using Weight = Solver::weight_type;
using Cell = grid_cell;
using i64 = std::int64_t;
using u8 = std::uint8_t;

std::vector<Summary> oracle(int h, int w, const std::vector<Cell>& cells,
                            const std::vector<Weight>& weights, const std::vector<u8>& active) {
    const int n = h*w;
    std::vector<int> group(n,-1);
    unsigned next=0;
    for(int p=0;p<n;++p) {
        if(cells[p]==Cell::blocked || (cells[p]==Cell::optional && !active[next++])) group[p]=-2;
    }
    assert(next==active.size());
    std::vector<Summary> out;
    for(int p=0;p<n;++p) {
        if(group[p]!=-1) continue;
        const int id=static_cast<int>(out.size());
        group[p]=id;
        // Independent repeated connectivity relaxation, no contraction or
        // production neighbor traversal.
        bool changed=true;
        while(changed) {
            changed=false;
            for(int a=0;a<n;++a) {
                if(group[a]!=id)continue;
                for(int b=0;b<n;++b) {
                    if(group[b]==-1&&std::abs(a/w-b/w)+std::abs(a%w-b%w)==1) {
                        group[b]=id;changed=true;
                    }
                }
            }
        }
        Summary x{};
        for(int a=0;a<n;++a) if(group[a]==id) {++x.cells;for (int d=0;d<2;++d) x.weight[d]+=weights[a][d];}
        out.push_back(x);
    }
    return out;
}
void norm(std::vector<Summary>& v){
    std::sort(v.begin(),v.end(),[](auto a,auto b){return std::pair(a.cells,a.weight)<std::pair(b.cells,b.weight);});
}
void verify(const Solver& solver,int h,int w,const std::vector<Cell>& a,
            const std::vector<Weight>& b,const std::vector<u8>& q) {
    auto x=solver.affected(q);
    auto y=oracle(h,w,a,b,q);
    x.insert(x.end(),solver.unaffected().begin(),solver.unaffected().end());
    norm(x);norm(y);assert(x==y);
    std::size_t k=0, count=0;Weight weight{};
    for(std::size_t p=0;p<a.size();++p) {
        if(a[p]==Cell::optional) assert(solver.optional_cells()[k]==p);
        if(a[p]==Cell::fixed_open || (a[p]==Cell::optional && q[k++])) {
            ++count;for (int d=0;d<2;++d) weight[d]+=b[p][d];
        }
    }
    const auto s=std::accumulate(x.begin(),x.end(),Summary{},[](Summary u,Summary v) {
        return Summary{u.cells+v.cells,{u.weight[0]+v.weight[0],u.weight[1]+v.weight[1]}};
    });
    assert(s.cells==count&&s.weight==weight);
}
int main(){
    assert(optional_grid_components<>(0,5,{}).affected({}).empty());
    std::size_t checks=0;
    for(const auto& [h,w]: std::array{std::pair{1,1},std::pair{1,6},std::pair{2,3},std::pair{3,2}}){
        int states=1;for(int i=0;i<h*w;++i)states*=3;
        for(int s=0;s<states;++s){
            std::vector<Cell> a(h*w); std::vector<Weight>b(h*w);
            auto x=s;
            for(int j=0;j<h*w;++j){
                a[j]=static_cast<Cell>(x%3);x/=3;
                b[j]={j%2?-7:31-j,j%3==0?-13:j*j+1};
            }
            const Solver solver(h,w,a,b);
            const auto k=solver.optional_cells().size();
            for(unsigned m=0;m<(1u<<k);++m){
                std::vector<u8> q(k);for(unsigned j=0;j<k;++j)q[j]=(m>>j)&1;
                verify(solver,h,w,a,b,q);++checks;
            }
        }
    }
    // Adjacent optionals are a bridge; a separate unaffected weighted
    // component appears only in unaffected(), even with mask all false.
    {
        const std::vector<Cell> a={Cell::fixed_open,Cell::optional,Cell::optional,Cell::fixed_open,
                                   Cell::blocked,Cell::fixed_open};
        const std::vector<Weight> b={{{7,-6}},{{2,8}},{{3,3}},{{-5,0}},{{100,-25}},{{11,44}}};
        Solver s(1,6,a,b);
        assert(s.unaffected().size()==1 && s.unaffected()[0]==(Summary{1,{11,44}}));
        assert(s.affected(std::array<u8,2>{1,1})==std::vector<Summary>({{4,{7,5}}}));
        assert(s.affected(std::array<u8,2>{0,0}).size()==2);
        bool ex=false;try{(void)s.affected(std::array<u8,2>{0,2});}catch(const std::invalid_argument&){ex=true;}
        assert(ex);
        ex=false;try{(void)s.affected(std::array<u8,1>{1});}catch(const std::invalid_argument&){ex=true;}
        assert(ex);
    }
    std::mt19937 rng(20261123);
    for(int i=0;i<500;++i){
        const int h=1+rng()%7,w=1+rng()%9;
        std::vector<Cell>a(h*w);for(auto&c:a)c=static_cast<Cell>(rng()%3);
        std::vector<Weight>b(h*w);for(auto&v:b)for(auto& z:v)z=static_cast<int>(rng()%201)-100;
        Solver s(h,w,a,b);
        for(int j=0;j<8;++j){
            std::vector<u8> q(s.optional_cells().size());for(auto&v:q)v=rng()%2;
            verify(s,h,w,a,b,q);++checks;
        }
    }
    // No machine word cap. Mask bits beyond 64 can alter reachability.
    {
        std::vector<Cell>a(131,Cell::optional);std::vector<Weight>b(131,Weight{3,-5});
        Solver s(1,131,a,b);
        std::vector<u8> q(131,1);q[65]=0;
        verify(s,1,131,a,b,q);++checks;
        const auto out=s.affected(q);
        assert(out.size()==2 && out[0].cells==65 && out[1].cells==65);
    }
    {
        const std::array a{Cell::fixed_open,Cell::optional,Cell::fixed_open};
        optional_grid_components<> count_only(1,3,a);
        const auto out=count_only.affected(std::array<u8,1>{1});
        assert(out.size()==1&&out[0].cells==3&&out[0].weight.empty());
        bool ex=false;
        try {(void)Solver(1,3,a);}catch(const std::invalid_argument&){ex=true;}
        assert(ex);
    }
    std::cout<<"verified queries "<<checks<<'\n';
}
