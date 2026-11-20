#pragma GCC optimize("O3")
#include <algorithm>
#include <cmath>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <set>
#include <tuple>
#include <utility>
#include <vector>
#include "../../library/hilbert-order.hpp"
#if !defined(AHC045_DISCOUNT_ONLY) && !defined(AHC045_ORDINAL)
#define AHC045_ORDINAL
#endif
#ifdef AHC045_ORDINAL
#include "../../library/bounded-order-projection.hpp"
#endif

struct Box {
    int lx,rx,ly,ry;
    int x() const {return (lx+rx)/2;}
    int y() const {return (ly+ry)/2;}
};

using Edges=std::vector<std::pair<int,int>>;
struct Model {
    int n,m,budget,limit,width,used=0;
    std::vector<int> sizes;
    std::vector<Box> pos;
    std::vector<float> cost,lower,upper;
    std::vector<unsigned char> hit;
    std::vector<std::pair<int,int>> inequalities;
    int edge(int a,int b) const {if(a>b) std::swap(a,b);return a*n+b;}
    float length(int a,int b) const {return cost[edge(a,b)];}
    void measure() {
        cost.resize(n*n);lower.resize(n*n);upper.resize(n*n);hit.resize(n*n);
        for(int a=0;a<n;++a) for(int b=a+1;b<n;++b) {
            const auto& p=pos[a];const auto& q=pos[b];
            auto norm=[](int x,int y) {return std::sqrt(double(1LL*x*x+1LL*y*y));};
            int k=edge(a,b);
            lower[k]=float(std::floor(norm(std::max({0,p.lx-q.rx,q.lx-p.rx}),
                                     std::max({0,p.ly-q.ry,q.ly-p.ry}))));
            upper[k]=float(std::floor(norm(std::max(p.rx-q.lx,q.rx-p.lx),
                                     std::max(p.ry-q.ly,q.ry-p.ly))));
            cost[k]=std::max(lower[k],std::min(upper[k],float(norm(p.x()-q.x(),p.y()-q.y()))));
        }
    }
    std::pair<double,Edges> mst(const std::vector<int>& group,bool export_edges) const {
        const int count=group.size();
        std::vector<float> key(count,std::numeric_limits<float>::infinity());
        std::vector<int> parent(count,-1);
        std::vector<bool> used(count,false);
        Edges result;
        if(export_edges) result.reserve(count-1);
        key[0]=0;
        double sum=0;
        for(int j=0;j<count;++j) {
            int at=-1;
            for(int i=0;i<count;++i) if(!used[i] && (at<0 || key[i]<key[at])) at=i;
            used[at]=true;sum+=key[at];
            if(j && export_edges) result.emplace_back(group[parent[at]],group[at]);
            for(int i=0;i<count;++i) if(!used[i]) {
                float z=length(group[at],group[i]);
                if(z<key[i]) key[i]=z,parent[i]=at;
            }
        }
        return {sum,std::move(result)};
    }
    std::vector<std::vector<int>> groups() const {
        std::vector<std::vector<int>> best;
        double value=std::numeric_limits<double>::infinity();
        for(int mode=0;mode<16;++mode) {
            std::vector<std::pair<std::uint64_t,int>> sorted;
            for(int j=0;j<n;++j) {
                int a=pos[j].x(),b=pos[j].y();
                if(mode&1) std::swap(a,b);
                if(mode&2) a=10000-a;
                if(mode&4) b=10000-b;
                sorted.emplace_back(hilbert_order_2d(a,b,14),j);
            }
            std::sort(sorted.begin(),sorted.end());
            std::vector<int> group_order(m);
            std::iota(group_order.begin(),group_order.end(),0);
            if(mode&8) std::stable_sort(group_order.begin(),group_order.end(),[&](int a,int b) {
                return sizes[a]>sizes[b];
            });
            std::vector<std::vector<int>> own(m);
            double total=0;
            int k=0;
            for(int g:group_order) {
                auto& row=own[g];
                for(int j=0;j<sizes[g];++j) row.push_back(sorted[k++].second);
                total+=mst(row,false).first;
            }
            if(total<value) value=total,best=std::move(own);
        }
        return best;
    }
    Edges query(const std::vector<int>& part,bool train) {
        ++used;
        std::cout<<"? "<<part.size();
        for(int x:part) std::cout<<' '<<x;
        std::cout<<'\n'<<std::flush;
        const int l=part.size();
        Edges answer;
        for(int j=1,a,b;j<l;++j) {
            if(!(std::cin>>a>>b) || a<0 || b<0) std::exit(1);
            answer.emplace_back(a,b);
            if(train) hit[edge(a,b)]=1;
        }
#ifdef AHC045_ORDINAL
        if(train) {
            std::vector<std::vector<std::pair<int,int>>> branching(l);
            for(auto [a,b]:answer) {
                int i=std::find(part.begin(),part.end(),a)-part.begin();
                int j=std::find(part.begin(),part.end(),b)-part.begin();
                int k=edge(a,b);
                branching[i].emplace_back(j,k);branching[j].emplace_back(i,k);
            }
            for(int from=0;from<l;++from) {
                std::vector<int> path;
                auto visit=[&](auto&& self,int v,int p) -> void {
                    if(v>from && path.size()>1)
                        for(int e:path) inequalities.emplace_back(edge(part[from],part[v]),e);
                    for(auto [u,e]:branching[v]) if(u!=p) {
                        path.push_back(e);self(self,u,v);path.pop_back();
                    }
                };
                visit(visit,from,-1);
            }
        }
#else
        (void)train;
#endif
        return answer;
    }
    void calibrate() {
        for(int i=0;i<n*n;++i) if(hit[i]) cost[i]=std::max(lower[i],.87f*cost[i]);
#ifdef AHC045_ORDINAL
        std::sort(inequalities.begin(),inequalities.end());
        inequalities.erase(std::unique(inequalities.begin(),inequalities.end()),inequalities.end());
        project_bounded_order(cost,lower,upper,inequalities,12);
#endif
    }
    void fit(const std::vector<std::vector<int>>& group,std::vector<Edges>& known) {
        std::set<std::vector<int>> seen;
        std::vector<std::vector<int>> windows;
        for(int g=0;g<m;++g) {
            const auto& c=group[g];
            if(c.size()>=3 && int(c.size())<=limit && used<budget) known[g]=query(c,false);
            if(int(c.size())<=limit) continue;
            for(int v:c) {
                std::vector<int> near=c;
                std::partial_sort(near.begin(),near.begin()+limit,near.end(),[&](int a,int b) {
                    return std::pair(length(v,a),a)<std::pair(length(v,b),b);
                });
                near.resize(limit);
                std::sort(near.begin(),near.end());
                if(seen.insert(near).second) windows.push_back(std::move(near));
            }
        }
        std::vector<int> freq(n);
        while(used<budget && !windows.empty()) {
            int best=-1,idx=-1;
            for(int i=0;i<int(windows.size());++i) {
                int score=0;
                for(int v:windows[i]) score+=512/(1+freq[v]);
                if(score>best) best=score,idx=i;
            }
            for(int v:windows[idx]) ++freq[v];
            query(windows[idx],true);
            windows[idx]=std::move(windows.back());
            windows.pop_back();
        }
        calibrate();
    }
#ifdef AHC045_POST_QUERY_SWAP
    void refine(std::vector<std::vector<int>>& group,const std::vector<Edges>& exact) const {
        // Queried whole-group MSTs remain exact and are never invalidated.
        std::vector<int> belong(n,-1),index(n,-1),available;
        std::vector<double> value(m);
        int open_groups=0;
        for(int g=0;g<m;++g) if(exact[g].empty()) {
            ++open_groups;
            value[g]=mst(group[g],false).first;
            for(int j=0;j<int(group[g].size());++j) {
                const int v=group[g][j];
                belong[v]=g;index[v]=j;available.push_back(v);
            }
        }
        if(open_groups<2) return;
        std::vector<std::vector<int>> near(n);
        const auto stop=std::chrono::steady_clock::now()+std::chrono::milliseconds(650);
        for(int u:available) {
            auto q=available;
            int k=std::min(24,int(q.size()));
            std::partial_sort(q.begin(),q.begin()+k,q.end(),[&](int a,int b) {
                return std::pair(length(u,a),a)<std::pair(length(u,b),b);
            });
            q.resize(k);
            near[u]=std::move(q);
        }
        uint32_t rng=758425039;
        auto pick=[&](int bound) {
            rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
            return rng%bound;
        };
        int work=0;
        for(int trial=0;trial<8192 && work<26'000'000;++trial) {
            if(trial%64==0 && std::chrono::steady_clock::now()>=stop) break;
            int u=available[pick(available.size())],v=near[u][pick(near[u].size())];
            int a=belong[u],b=belong[v];
            if(a==b) continue;
            auto& x=group[a];auto& y=group[b];
            int i=index[u],j=index[v];
            std::swap(x[i],y[j]);
            const auto left=mst(x,false).first,right=mst(y,false).first;
            work+=int(x.size()*x.size()+y.size()*y.size());
            if(left+right+1e-6<value[a]+value[b]) {
                value[a]=left;value[b]=right;
                std::swap(belong[u],belong[v]);
                std::swap(index[u],index[v]);
            } else std::swap(x[i],y[j]);
        }
    }
#endif

};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Model model;
    if(!(std::cin>>model.n>>model.m>>model.budget>>model.limit>>model.width)) return 1;
    model.sizes.resize(model.m);
    model.pos.resize(model.n);
    for(auto& i:model.sizes) std::cin>>i;
    for(auto& p:model.pos) std::cin>>p.lx>>p.rx>>p.ly>>p.ry;
    model.measure();
    auto group=model.groups();
    std::vector<Edges> known(model.m);
    model.fit(group,known);
#ifdef AHC045_POST_QUERY_SWAP
    model.refine(group,known);
#endif
    std::cout<<"!\n";
    for(int g=0;g<model.m;++g) {
        const auto& c=group[g];
        for(int i=0;i<int(c.size());++i) std::cout<<c[i]<<(i+1==int(c.size())?'\n':' ');
        if(known[g].empty()) known[g]=model.mst(c,true).second;
        for(auto [a,b]:known[g]) std::cout<<a<<' '<<b<<'\n';
    }
    std::cout<<std::flush;
}
