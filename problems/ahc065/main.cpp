#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>
#include "../../library/scattered-cycle-rotation.hpp"

struct RNG {
    uint32_t x=8398014;
    uint32_t next() {x^=x<<13;x^=x>>17;x^=x<<5;return x;}
};
struct State {
    std::array<int,400> board,position;
    int next=0;
};
struct Operation {
    int belt,direction,to;
};
struct Problem {
    int n,door;
    std::vector<std::vector<int>> belts;
    std::array<std::vector<Operation>,400> toward;
    std::array<int,400> dist;
    State start;
    void clear(State& s) const {
        if(s.board[door]==s.next) {
            s.board[door]=-1;
            s.position[s.next]=-1;
            ++s.next;
        }
    }
    bool init() {
        if(!(std::cin>>n)) return false;
        door=n/2;
        for(int i=0;i<n*n;++i) {
            std::cin>>start.board[i];
            start.position[start.board[i]]=i;
            dist[i]=i/n+std::abs(i%n-door);
        }
        clear(start);
        for(int x=0;x<n;x+=2) {
            std::vector<int> ring;
            for(int y=0;y<n;++y) ring.push_back(x*n+y);
            for(int y=n-1;y>=0;--y) ring.push_back((x+1)*n+y);
            belts.push_back(std::move(ring));
        }
        for(int y=0;y<n;y+=2) {
            std::vector<int> ring;
            for(int x=0;x<n;++x) ring.push_back(x*n+y);
            for(int x=n-1;x>=0;--x) ring.push_back(x*n+y+1);
            belts.push_back(std::move(ring));
        }
        for(int b=0;b<int(belts.size());++b) {
            const auto& ring=belts[b];
            for(int i=0;i<int(ring.size());++i) for(int d:{-1,1}) {
                int now=ring[i],to=ring[(i+d+ring.size())%ring.size()];
                if(dist[to]+1==dist[now]) toward[now].push_back({b,d,to});
            }
        }
        return true;
    }
    void apply(State& s,const Operation& a) const {
        rotate_scattered_cycle(s.board,belts[a.belt],a.direction,
            [&](int at,int box){if(box>=0) s.position[box]=at;});
        clear(s);
    }
    int priority(const State& s) const {
        int result=0;
#ifdef AHC065_ONE_FUTURE
        constexpr std::array<int,1> weight{16};
#else
        constexpr std::array<int,8> weight{16,13,10,8,6,5,3,2};
#endif
        for(int i=0;i<int(weight.size()) && s.next+i<n*n;++i)
            result+=weight[i]*dist[s.position[s.next+i]];
        return result;
    }
    std::vector<Operation> solve() const {
        std::vector<Operation> result;
        State current=start;
        RNG rand;
        for(;current.next<n*n;) {
            const int target=current.next;
            int best=std::numeric_limits<int>::max();
            State winner=current;
            std::vector<Operation> selected,path;
            for(int run=0;run<8;++run) {
                State copy=current;
                path.clear();
                while(copy.next==target) {
                    const auto& choice=toward[copy.position[target]];
                    const auto& op=choice[rand.next()%choice.size()];
                    apply(copy,op);path.push_back(op);
                }
                const int score=priority(copy);
                if(score<best) best=score,selected=path,winner=copy;
            }
            result.insert(result.end(),selected.begin(),selected.end());
            current=winner;
        }
        return result;
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Problem p;
    if(!p.init()) return 0;
    const auto ops=p.solve();
    std::cout<<p.belts.size()<<'\n';
    for(auto& belt:p.belts) {
        std::cout<<belt.size();
        for(int at:belt) std::cout<<' '<<at/p.n<<' '<<at%p.n;
        std::cout<<'\n';
    }
    std::cout<<ops.size()<<'\n';
    for(auto op:ops) std::cout<<op.belt<<' '<<op.direction<<'\n';
    std::cerr<<"Turns = "<<ops.size()<<'\n';
}
