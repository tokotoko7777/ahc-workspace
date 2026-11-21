#pragma GCC optimize("O3,unroll-loops")
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include "../../library/weighted-bottleneck-witness.hpp"

namespace {
constexpr int N = 400, M = 1995, SCENARIOS = 96, FIRST_BATCH = 12;
// A future minimum-cost choice (alternatives) makes the witness draw cheap;
// a bottleneck in a series of required links makes it expensive. Online
// opportunity loss moves thresholds in opposite directions for the two.
double risk(double mean, double sd, double div, int used, int sum_witness_distance) {
    const double slack=2.-used*mean/std::max(1,sum_witness_distance);
    return 0.24*sd*div*std::clamp(3.*slack,-1.75,2.2);
}
struct Edge { std::int16_t from, to, cost, id; };

struct Forest {
    std::array<int,N> parents;
    Forest() { parents.fill(-1); }
    int leader(int i) {
        while (parents[i] >= 0) {
            const int j = parents[i];
            if (parents[j] < 0) return j;
            i = parents[i] = parents[j];
        }
        return i;
    }
    bool join(int a,int b) {
        a=leader(a); b=leader(b);
        if(a==b) return false;
        if(parents[a]>parents[b]) std::swap(a,b);
        parents[a]+=parents[b]; parents[b]=a;
        return true;
    }
};

using Scenario=std::array<std::uint16_t,M>;
std::vector<Scenario> build_scenarios(const std::vector<Edge>& edges) {
    std::mt19937 rng(271907);
    // Keep the random stream of the frozen comparison variant. These old
    // scenario draws are superseded by the stratified lengths below.
    for(int k=0;k<SCENARIOS;++k)
        for(const auto& e:edges)
            (void)std::uniform_int_distribution<int>(e.cost,3*e.cost)(rng);
    std::vector<Scenario> out(SCENARIOS);
    std::array<int,SCENARIOS> perm;
    std::iota(perm.begin(),perm.end(),0);
    for(int j=0;j<M;++j) {
        const int d=edges[j].cost;
        std::shuffle(perm.begin(),perm.end(),rng);
        for(int k=0;k<SCENARIOS;++k) {
            const std::uint64_t f=(std::uint64_t(perm[k])<<32)+rng();
            out[k][j]=static_cast<std::uint16_t>(d+((f*(2*d+1)/SCENARIOS)>>32));
        }
    }
    return out;
}
}

int main() {
    const auto start=std::chrono::steady_clock::now();
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::array<int,N> x,y;
    for(int i=0;i<N;++i) if(!(std::cin>>x[i]>>y[i])) return 1;
    std::vector<Edge> edges;
    edges.reserve(M);
    for(int i=0;i<M;++i) {
        int u,v;
        if(!(std::cin>>u>>v)) return 1;
        const int dx=x[u]-x[v],dy=y[u]-y[v];
        edges.push_back({std::int16_t(u),std::int16_t(v),std::int16_t(std::lround(std::sqrt(dx*dx+dy*dy))),std::int16_t(i)});
    }
    const auto draws=build_scenarios(edges);
    BidirBottleneck<N,M> predictor(edges);
    Forest accepted;
    std::array<int,M> epoch{}, frequency{};
    bool groups_dirty=true;
    int num_accepted=0, queries=0, samples=0, early_stops=0, fallback=0;
    for(int i=0;i<M;++i) {
        const auto [u,v,_,edge_id]=edges[i];
        int current;
        if(!(std::cin>>current)) return 1;
        predictor.erase_next(i,u,v);
        bool take=false;
        if(accepted.leader(u)!=accepted.leader(v)) {
            ++queries;
            if(groups_dirty) {
                predictor.set_groups(accepted);
                groups_dirty=false;
            }
            const auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
            const int budget=elapsed>1.6 ? 1 : elapsed>1.45 ? 8:SCENARIOS;
            fallback+=budget<SCENARIOS;
            int used=0, sum=0, sq=0, sum_witness_distance=0, colliding_pairs=0, next_checkpoint=FIRST_BATCH;
            double mean=0, deviation=0, diversity=0;
            for(int k=0;k<budget;++k) {
                ++samples;
                int witness=-1;
                int limit=predictor.cutoff_active(draws[k],u,v,&witness);
                // Future connectivity is independent of all lengths.
                if(limit<0) {take=true;break;}
                if(epoch[witness]!=i+1) {
                    epoch[witness]=i+1;
                    frequency[witness]=0;
                }
                colliding_pairs+=frequency[witness]++;
                sum+=limit;
                sq+=limit*limit;
                sum_witness_distance+=edges[witness].cost;
                ++used;
                if(used==next_checkpoint && used<budget) {
                    mean=double(sum)/used;
                    deviation=std::sqrt(std::max(0.,sq/double(used)-mean*mean));
                    diversity=1.-2.*colliding_pairs/(used*(used-1.));
                    const double estimate=mean+risk(mean,deviation,diversity,used,sum_witness_distance);
                    const double band=2.5*deviation/std::sqrt(used)+0.015*mean;
                    if(std::abs(current-estimate)>band) {
                        ++early_stops;
                        break;
                    }
                    next_checkpoint*=2;
                }
            }
            if(!take) {
                mean=sum/double(used);
                deviation=std::sqrt(std::max(0.,sq/double(used)-mean*mean));
                diversity=used<=1?0.:1.-2.*colliding_pairs/(used*(used-1.));
                take=current<=mean+risk(mean,deviation,diversity,used,sum_witness_distance);
            }
            if(take) {
                accepted.join(u,v);
                groups_dirty=true;
                ++num_accepted;
            }
        }
        std::cout<<take<<'\n'<<std::flush;
    }
    std::cerr<<"nqueries="<<queries<<" samples="<<samples<<" short="<<early_stops
             <<" fallback="<<fallback<<'\n';
    return num_accepted!=N-1;
}
