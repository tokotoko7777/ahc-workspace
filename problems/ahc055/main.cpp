#pragma GCC optimize("O3")
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>

using Clock=std::chrono::steady_clock;
struct RNG {
    uint32_t value=552883;
    uint32_t next() {
        value^=value<<13;value^=value>>17;value^=value<<5;
        return value;
    }
    double unit() {return (next()+.5)/4294967296.;}
};
struct Model {
    int n;
    std::vector<int> hardness,capacity,owner,begin;
    std::vector<std::vector<int>> damage,pref;
    void read() {
        std::cin>>n;
        hardness.resize(n);capacity.resize(n);
        for(int& a:hardness) std::cin>>a;
        begin.resize(n);
        for(int w=0;w<n;++w) {
            std::cin>>capacity[w];
            begin[w]=owner.size();
            for(int j=0;j<capacity[w];++j) owner.push_back(w);
        }
        damage.assign(n,std::vector<int>(n));
        for(auto& row:damage) for(int& a:row) std::cin>>a;
        pref.assign(n,std::vector<int>(n));
        for(int b=0;b<n;++b) {
            std::iota(pref[b].begin(),pref[b].end(),0);
            std::stable_sort(pref[b].begin(),pref[b].end(),[&](int a,int c) {
                return damage[a][b]>damage[c][b];
            });
        }
    }
    std::vector<int> greedy(std::vector<int>& allocation) const {
        std::vector<int> order,charge(n,-1);
        std::vector<int> quality(n);
        for(int i=0;i<n;++i) {
            auto row=damage[i];
            std::nth_element(row.begin(),row.begin()+std::min(5,n-1),row.end(),std::greater<int>());
            for(int j=0;j<std::min(n,5);++j) quality[i]+=row[j];
            quality[i]*=capacity[i];
        }
        for(int step=0;step<n;++step) {
            int box=-1,score=std::numeric_limits<int>::max();
            for(int b=0;b<n;++b) if(charge[b]<0) {
                int rem=hardness[b],hits=0;
                for(int w:pref[b]) {
                    const int a=damage[w][b];
                    if(a<=1 || rem<=1) break;
                    if(charge[w]<=0) continue;
                    const int use=std::min(charge[w],(rem+a-1)/a);
                    rem-=use*a;
                    hits+=use;
                }
                hits+=std::max(0,rem);
                if(hits<score || (hits==score && quality[b]>quality[box]))
                    box=b,score=hits;
            }
            int rem=hardness[box];
            for(int w:pref[box]) {
                const int a=damage[w][box];
                if(a<=1 || rem<=1) break;
                while(charge[w]>0 && rem>1) {
                    --charge[w];
                    rem-=a;
                    allocation[begin[w]+charge[w]]=box;
                }
            }
            charge[box]=capacity[box];
            order.push_back(box);
        }
        return order;
    }
    int strikes(int cell,const std::vector<int>& slots) const {
        if(cell<0) return 0;
        int sum=0;
        for(int s:slots) sum+=damage[owner[s]][cell];
        if(sum<hardness[cell]) return hardness[cell]-sum+slots.size();
        std::vector<int> values;
        values.reserve(slots.size());
        for(int s:slots) values.push_back(damage[owner[s]][cell]);
        std::sort(values.begin(),values.end(),std::greater<int>());
        int remain=hardness[cell],shots=0;
        for(int d:values) {
            if(remain<=1) break;
            ++shots;remain-=d;
        }
        return shots+std::max(0,remain);
    }
    std::vector<std::vector<int>> group(const std::vector<int>& alloc) const {
        std::vector<std::vector<int>> buckets(n+1);
        for(int slot=0;slot<int(alloc.size());++slot)
            buckets[alloc[slot]<0?n:alloc[slot]].push_back(slot);
        return buckets;
    }
    void improve(const std::vector<int>& order,std::vector<int>& alloc,Clock::time_point deadline) const {
        std::vector<int> rank(n);
        for(int i=0;i<n;++i) rank[order[i]]=i;
        std::vector<std::vector<int>> favored(n);
        for(int w=0;w<n;++w) {
            auto& f=favored[w];
            for(int b=0;b<n;++b) if(rank[b]>rank[w]) f.push_back(b);
            const int best=std::min(18,int(f.size()));
            std::partial_sort(f.begin(),f.begin()+best,f.end(),[&](int a,int b) {
                return std::pair(std::min(hardness[a],damage[w][a]),a)>
                       std::pair(std::min(hardness[b],damage[w][b]),b);
            });
            f.resize(best);
        }
        auto piles=group(alloc);
        std::vector<int> value(n);
        int total=0;
        for(int b=0;b<n;++b) total+=value[b]=strikes(b,piles[b]);
        int best_total=total;
        std::vector<int> best=alloc;
        RNG rng;
        constexpr int steps=170000;
        double temp=12.;
        auto idx=[&](int box){return box<0 ? n : box;};
        for(int i=0;i<steps;++i) {
            if(i%256==0) {
                if(Clock::now()>=deadline) break;
                temp=12.*std::pow(.017,double(i)/steps);
            }
            const int slot=rng.next()%alloc.size(),w=owner[slot];
            if(favored[w].empty()) continue;
            const unsigned mode=rng.next()%16;
            const int to=mode==0 ? -1 :
                mode<13 ? favored[w][rng.next()%favored[w].size()] :
                order[rank[w]+1+rng.next()%(n-1-rank[w])];
            const int from=alloc[slot];
            if(to==from) continue;
            auto& a=piles[idx(from)];auto& b=piles[idx(to)];
            auto at=std::find(a.begin(),a.end(),slot);
            std::iter_swap(at,a.end()-1);
            a.pop_back();b.push_back(slot);
            const int first=strikes(from,a),second=strikes(to,b);
            const int previous=(from>=0?value[from]:0)+(to>=0?value[to]:0);
            const int gain=previous-first-second;
            if(gain>=0 || rng.unit()<std::exp(gain/temp)) {
                if(from>=0) value[from]=first;
                if(to>=0) value[to]=second;
                alloc[slot]=to;
                total-=gain;
                if(total<best_total) best_total=total,best=alloc;
            } else {
                b.pop_back();a.push_back(slot);
            }
        }
        alloc.swap(best);
    }
    std::vector<std::pair<int,int>> commands(const std::vector<int>& order,
                                             const std::vector<int>& alloc) const {
        auto parts=group(alloc);
        std::vector<std::pair<int,int>> result;
        for(int b:order) {
            int remain=hardness[b];
            std::sort(parts[b].begin(),parts[b].end(),[&](int a,int c) {
                return std::pair(damage[owner[a]][b],a)>std::pair(damage[owner[c]][b],c);
            });
            for(int shot:parts[b]) {
                if(remain<=1) break;
                result.emplace_back(owner[shot],b);
                remain-=damage[owner[shot]][b];
            }
            for(;remain>0;--remain) result.emplace_back(-1,b);
        }
        return result;
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    const auto deadline=Clock::now()+std::chrono::milliseconds(1650);
    Model model;
    model.read();
    std::vector<int> allocation(model.owner.size(),-1);
    auto order=model.greedy(allocation);
#ifndef AHC055_GREEDY_ONLY
    model.improve(order,allocation,deadline);
#endif
    auto result=model.commands(order,allocation);
    for(auto [w,b]:result) std::cout<<w<<' '<<b<<'\n';
    std::cerr<<"Model = "<<std::accumulate(model.hardness.begin(),model.hardness.end(),1LL)-result.size()<<'\n';
}
