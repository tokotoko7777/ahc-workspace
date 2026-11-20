#include "../library/bounded-order-projection.hpp"
#include <cmath>
#include <random>

int main() {
    std::mt19937 rng(4504801);
    for(int trial=0;trial<1500;++trial) {
        std::vector<double> lo(2),hi(2),value(2);
        for(int j=0;j<2;++j) {
            lo[j]=rng()%18;hi[j]=lo[j]+double(rng()%12);
            value[j]=lo[j]+double(rng()%1001)/1000*(hi[j]-lo[j]);
        }
        const auto initial=value;
        project_bounded_order(value,lo,hi,{{0,1}},1);
        for(int j=0;j<2;++j) assert(lo[j]<=value[j] && value[j]<=hi[j]);
        if(hi[0]<lo[1]) {assert(value==initial);continue;}
        assert(value[0]>=value[1]);
        auto cost=[&](double a,double b) {
            return std::pow(a-initial[0],2)+std::pow(b-initial[1],2);
        };
        for(int a=lo[0];a<=hi[0];++a)
            for(int b=lo[1];b<=hi[1];++b) if(a>=b)
                assert(cost(value[0],value[1])<=cost(a,b)+1e-8);
    }
    std::vector<float> value{0,3,6,9},lo(4),hi(4,10);
    project_bounded_order(value,lo,hi,{{0,1},{1,2},{2,3}},100);
    for(float x:value) assert(std::abs(x-4.5f)<1e-5f);
}
