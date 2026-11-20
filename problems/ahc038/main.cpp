#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

#if !defined(AHC038_STATIONS)
#define AHC038_NO_BEAM_EXAMPLE_MAIN
#include "../../examples/search/ahc038_variable_cost_beam.cpp"
#endif

namespace station {
using Clock=std::chrono::steady_clock;
constexpr int D=4,MAX_V=15;
constexpr std::array<int,D> dr{0,1,0,-1},dc{1,0,-1,0};
struct Board {
    int n,items,limit;
    std::vector<std::string> a,b;
};
struct Answer {
    int r=0,c=0;
    bool complete=false;
    std::vector<std::string> commands;
};
struct Station {
    int id=-1,steps=0,picks=0,drops=0;
    double value=-1e99;
    std::array<int,MAX_V> option{};
};
struct Robot {
    const Board& input;
    int n,k,n2,r,c,unplaced=0;
    double drop_weight;
    std::array<int,MAX_V> direction{};
    std::array<bool,MAX_V> held{};
    std::vector<int> state;
    // Offboard entries are -1. A location's offset determines which leaf
    // can touch it; lengths 1..k make the per-station hands independent.
    const std::vector<std::array<std::array<int,4>,MAX_V>>& reachable;
    explicit Robot(const Board& p,int leaves,int mode,
                   const std::vector<std::array<std::array<int,4>,MAX_V>>& reach)
        :input(p),n(p.n),k(leaves),n2(n*n),r(0),c(0),
         drop_weight(1.04+.115*(mode%4)),state(n2),reachable(reach) {
        for(int i=0;i<n2;++i) {
            state[i]=input.a[i/n][i%n]-input.b[i/n][i%n];
            unplaced+=state[i]<0;
        }
    }
    static int rotations(int before,int after) {
        const int delta=(after-before+4)%4;
        return std::min(delta,4-delta);
    }
    int initial_root() const {
        int id_best=0,value_best=-1;
        for(int id=0;id<n2;++id) {
            int value=0;
            for(int j=0;j<k;++j)
                for(int dir:{0,1,3}) {
                    const int q=reachable[id][j][dir];
                    if(q>=0 && state[q]==1) {
                        value+=10+(dir==0);break;
                    }
                }
            if(value>value_best) id_best=id,value_best=value;
        }
        return id_best;
    }
    std::array<std::vector<int>,2> nearest() const {
        std::array<std::vector<int>,2> distance;
        for(int mode=0;mode<2;++mode) {
            auto& dist=distance[mode];
            dist.assign(n2,2*n);
            const int goal=mode?1:-1;
            for(int i=0;i<n2;++i) if(state[i]==goal) dist[i]=0;
            for(int i=0;i<n2;++i) {
                if(i>=n) dist[i]=std::min(dist[i],dist[i-n]+1);
                if(i%n) dist[i]=std::min(dist[i],dist[i-1]+1);
            }
            for(int i=n2-1;i>=0;--i) {
                if(i+n<n2) dist[i]=std::min(dist[i],dist[i+n]+1);
                if(i%n+1<n) dist[i]=std::min(dist[i],dist[i+1]+1);
            }
        }
        return distance;
    }
    Station pick() const {
        Station best;
        const auto distance=nearest();
        for(int id=0;id<n2;++id) {
            const int travel=std::abs(id/n-r)+std::abs(id%n-c);
            // Sorting by expected progress per turn makes far stations
            // provably unable to win once a sufficiently dense batch exists.
            if(travel>2 && k*std::max(1.,drop_weight)/travel+0.07<best.value) continue;
            const int low=std::max(travel,1), high=std::max(low,2);
            for(int steps=low;steps<=high;++steps) {
                Station candidate;
                candidate.id=id;candidate.steps=steps;candidate.option.fill(-1);
                int tie=0;
                for(int leaf=0;leaf<k;++leaf) {
                    const int want=held[leaf]?-1:1;
                    int best_dir=-1,best_key=1'000'000;
                    for(int dir=0;dir<D;++dir) {
                        int rotate=rotations(direction[leaf],dir);
                        if(rotate>steps) continue;
                        int cell=reachable[id][leaf][dir];
                        if(cell<0 || state[cell]!=want) continue;
                        int future=distance[held[leaf]?1:0][cell];
                        int key=4*future+rotate;
                        if(key<best_key) best_key=key,best_dir=dir;
                    }
                    if(best_dir<0) continue;
                    candidate.option[leaf]=best_dir;
                    tie+=best_key;
                    if(held[leaf]) ++candidate.drops;
                    else ++candidate.picks;
                }
                if(candidate.picks+candidate.drops==0) continue;
                candidate.value=(candidate.picks+drop_weight*candidate.drops)/double(steps)
                    + .004*std::min(7,candidate.picks+candidate.drops)
                    - .00012*tie/double(candidate.picks+candidate.drops);
                if(candidate.value>best.value) best=candidate;
            }
        }
        return best;
    }
    Answer solve(Clock::time_point deadline) {
        Answer a;
        const int start=initial_root();
        a.r=r=start/n; a.c=c=start%n;
        while(unplaced) {
            if(Clock::now()>=deadline) return a;
            const Station next=pick();
            if(next.id<0) return a;
            const int nr=next.id/n,nc=next.id%n;
            for(int step=0;step<next.steps;++step) {
                std::string record(2*(k+1),'.');
                auto vert=[&]() {
                    if(r==nr) return false;
                    const bool down=r<nr;
                    record[0]=down?'D':'U'; r+=down?1:-1; return true;
                };
                auto hori=[&]() {
                    if(c==nc) return false;
                    const bool right=c<nc;
                    record[0]=right?'R':'L'; c+=right?1:-1; return true;
                };
                if(!vert()) hori();
                for(int leaf=0;leaf<k;++leaf) if(next.option[leaf]>=0) {
                    const int want=next.option[leaf];
                    int diff=(want-direction[leaf]+4)%4;
                    if(diff==1||diff==2) record[leaf+1]='R',direction[leaf]=(direction[leaf]+1)%4;
                    else if(diff==3) record[leaf+1]='L',direction[leaf]=(direction[leaf]+3)%4;
                    if(step+1==next.steps) {
                        const int cell=reachable[next.id][leaf][want];
                        record[k+2+leaf]='P';
                        state[cell]=0;
                        if(held[leaf]) --unplaced;
                        held[leaf]=!held[leaf];
                    }
                }
                a.commands.push_back(std::move(record));
            }
        }
        a.complete=true;
        return a;
    }
};
Board read(std::istream& in) {
    Board p;
    if(!(in>>p.n>>p.items>>p.limit)) return {};
    p.a.resize(p.n);p.b.resize(p.n);
    for(auto& row:p.a) in>>row;
    for(auto& row:p.b) in>>row;
    return p;
}
Answer solve(const Board& p,Clock::time_point deadline,bool must_finish) {
    const int k=std::min(p.limit-1,p.n/2);
    std::vector<std::array<std::array<int,4>,MAX_V>> reach(p.n*p.n);
    for(int i=0;i<p.n*p.n;++i) for(int leaf=0;leaf<k;++leaf) for(int dir=0;dir<4;++dir) {
        const int r=i/p.n+(leaf+1)*dr[dir],c=i%p.n+(leaf+1)*dc[dir];
        reach[i][leaf][dir]=(0<=r&&r<p.n&&0<=c&&c<p.n)?r*p.n+c:-1;
    }
    Answer best;
    for(int mode=0;mode<4;++mode) {
        Robot candidate(p,k,mode,reach);
        auto plan=candidate.solve(mode==0 && must_finish ? Clock::time_point::max() : deadline);
        if(plan.complete && (!best.complete || plan.commands.size()<best.commands.size()))
            best=std::move(plan);
        if(Clock::now()>deadline) break;
    }
    return best;
}
}

int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
#ifdef AHC038_STATIONS
    const auto start=station::Clock::now();
    const auto p=station::read(std::cin);
    if(!p.n) return 1;
    auto best=station::solve(p,start+std::chrono::milliseconds(2200),true);
    if(!best.complete) return 1;
    const int k=std::min(p.limit-1,p.n/2);
    std::cout<<k+1<<'\n';
    for(int i=1;i<=k;++i) std::cout<<"0 "<<i<<'\n';
    std::cout<<best.r<<' '<<best.c<<'\n';
    for(auto& s:best.commands) std::cout<<s<<'\n';
    std::cerr<<"Model = "<<best.commands.size()<<'\n';
#else
    const std::string data{std::istreambuf_iterator<char>(std::cin),{}};
    std::istringstream beam_input(data);
    Ahc038BeamProblem problem;
    problem.read(beam_input);
#if defined(AHC038_HYBRID) || defined(AHC038_WORK_CONTROL)
    constexpr int steps=256;
#else
    constexpr int steps=-1;
#endif
#ifdef AHC038_HYBRID
    std::istringstream station_input(data);
    auto board=station::read(station_input);
    auto warm=station::solve(board,station::Clock::now()+std::chrono::milliseconds(60),false);
    Answer incumbent;
    if(warm.complete) {
        const int k=std::min(board.limit-1,board.n/2);
        incumbent.lengths.resize(k);
        std::iota(incumbent.lengths.begin(),incumbent.lengths.end(),1);
        incumbent.initial_root_row=warm.r;
        incumbent.initial_root_column=warm.c;
        auto& m=incumbent.moves.emplace_back();
        m.advance=warm.commands.size();
        m.commands=std::move(warm.commands);
    }
    const Answer answer=problem.solve(7,2100.0,warm.complete ? &incumbent : nullptr,steps);
#elif defined(AHC038_WORK_CONTROL)
    const Answer answer=problem.solve(7,2100.0,nullptr,steps);
#else
#ifndef AHC038_SEARCH_TIME_MS
#define AHC038_SEARCH_TIME_MS 2200.0
#endif
    const Answer answer=problem.solve(7,AHC038_SEARCH_TIME_MS);
#endif
    problem.print_answer(answer,std::cout);
    std::cerr<<"Model = "<<answer.turns()<<'\n';
#endif
}
