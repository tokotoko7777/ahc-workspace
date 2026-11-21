#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <cmath>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <numeric>
#include <vector>
#include "../../library/dsu.hpp"
#include "../../library/kruskal-bottleneck.hpp"
#ifdef AHC_SMALLBLOCK
#include "vertex_block.hpp"
#endif
#ifdef AHC_ACTIVE_SAMPLES
#include "active_samples.hpp"
#endif
#ifdef AHC_PIVOTAL
#include "pivotal_rollout.hpp"
#endif
#ifdef AHC_BIDIR_SAMPLES
#include "bidir_samples.hpp"
#endif
#ifdef AHC_FAST_SAMPLES
#include "fast_samples.hpp"
#endif

constexpr int N = 400, M = 1995;
struct Edge { int from, to, cost, id; };

#ifdef AHC_PAIRED_ROLLOUT
// Reveal only the current sampled weight to the rollout policy. After a
// short prefix, score the remainder with exact sampled MST completion.
int play_then_complete(Dsu dsu, int first, int end,
                       const std::vector<Edge>& edges, const std::vector<Edge>& mean_order,
                       const std::array<int, M>& weights, const std::vector<Edge>& sampled_order) {
    Dsu scratch(N);
    int total = 0;
    for (int j = first; j < end; ++j) {
        const auto& e = edges[j];
        if (dsu.same(e.from, e.to)) continue;
        scratch = dsu;
        const auto cutoff = kruskal_path_bottleneck(scratch, mean_order, e.from, e.to,
            [j](const Edge& other) { return other.id > j; });
        if (!cutoff || weights[j] <= *cutoff) {
            dsu.unite(e.from, e.to);
            total += weights[j];
        }
    }
    for (const auto& e : sampled_order) {
        if (dsu.component_count() == 1) break;
        if (e.id >= end && dsu.unite(e.from, e.to)) total += e.cost;
    }
    if (dsu.component_count() != 1) return -1;
    return total;
}
#endif

#ifdef AHC_MATCHED_REPLACEMENT
struct PrefixPlay {
    Dsu tree;
    int cost=0;
};
struct SuffixTree {
    int cost=0;
    std::vector<Edge> selected;
};
PrefixPlay play_prefix(Dsu dsu,int first,int end,const std::vector<Edge>& edges,
                       const std::vector<Edge>& mean_order,const std::array<int,M>& weights) {
    PrefixPlay out{std::move(dsu),0};
    Dsu scratch(N);
    for(int i=first;i<end;++i) {
        const auto& e=edges[i];
        if(out.tree.same(e.from,e.to)) continue;
        scratch=out.tree;
        const auto limit=kruskal_path_bottleneck(scratch,mean_order,e.from,e.to,
            [i](const Edge& other){return other.id>i;});
        if(!limit || weights[i]<=*limit) out.tree.unite(e.from,e.to),out.cost+=weights[i];
    }
    return out;
}
SuffixTree suffix(const PrefixPlay& prefix,int first,const std::vector<Edge>& sample) {
    Dsu tree=prefix.tree;
    SuffixTree out{prefix.cost,{}};
    for(const auto& e:sample) {
        if(tree.component_count()==1) break;
        if(e.id>=first && tree.unite(e.from,e.to)) out.cost+=e.cost,out.selected.push_back(e);
    }
    if(tree.component_count()!=1) out.cost=-1;
    return out;
}
// The final release on the unique sampled replacement path under contraction.
int final_release(Dsu contracted,const std::vector<Edge>& tree,int a,int b) {
    a=contracted.leader(a);b=contracted.leader(b);
    if(a==b) return -1;
    std::array<int,N> head,seen,queue;
    head.fill(-1);seen.fill(-2);
    int num=0;
    std::array<int,2*N> to,next,id;
    auto add=[&](int x,int y,int e) {
        to[num]=y;next[num]=head[x];id[num]=e;head[x]=num++;
    };
    for(auto e:tree) {
        int u=contracted.leader(e.from),v=contracted.leader(e.to);
        add(u,v,e.id);add(v,u,e.id);
    }
    int cur=0,last=1;queue[0]=a;seen[a]=-1;
    while(cur<last) {
        int x=queue[cur++];
        if(x==b) return seen[x];
        for(int i=head[x];i>=0;i=next[i]) if(seen[to[i]]==-2) {
            seen[to[i]]=std::max(seen[x],id[i]);
            queue[last++]=to[i];
        }
    }
    return -1;
}
int can_accept_at_release(const PrefixPlay& prefix,const SuffixTree& offline,int release,
                          const std::vector<Edge>& mean_order,const std::vector<Edge>& sample) {
    const auto e=std::find_if(offline.selected.begin(),offline.selected.end(),
                              [release](const Edge& x){return x.id==release;});
    if(e==offline.selected.end()) return offline.cost;
    PrefixPlay before=prefix;
    for(const auto& x:offline.selected) if(x.id<release && before.tree.unite(x.from,x.to))
        before.cost+=x.cost;
    Dsu scratch=before.tree;
    const auto threshold=kruskal_path_bottleneck(scratch,mean_order,e->from,e->to,
        [release](const Edge& x){return x.id>release;});
    if(!threshold || e->cost<=*threshold) return offline.cost;
    const auto late=suffix(before,release+1,sample);
    if(late.cost<0) return offline.cost;
    return std::max(offline.cost,late.cost);
}
#endif

int main() {
#ifdef AHC_PAIRED_ROLLOUT
    const auto started = std::chrono::steady_clock::now();
    // Stop added search early and leave margin for the cheap valid fallback.
    const auto rollout_deadline = started + std::chrono::milliseconds(1350);
    const auto scenario_deadline = started + std::chrono::milliseconds(1550);
    int rollouts = 0, changes = 0;
#ifdef AHC_MATCHED_REPLACEMENT
    int targeted=0, late_changes=0;
#endif
#endif
#ifdef AHC_ADAPTIVE
    const auto anneal_started = std::chrono::steady_clock::now();
    const auto sample_deadline = anneal_started + std::chrono::milliseconds(1480);
    int query_count=0, sample_count=0, short_count=0, fallback_count=0;
#ifdef AHC_SMALLBLOCK
    int blocks_considered=0, blocks_exact=0, blocks_changed=0;
#endif
#ifdef AHC_PIVOTAL
    const auto rollout_deadline = anneal_started + std::chrono::milliseconds(1300);
    int pivotal_count=0, pivotal_changed=0, pivotal_total_adjust=0;
#ifdef AHC_FOCUS_CUT
    int focused=0;
#endif
#endif
#endif
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::array<int, N> x{}, y{};
    for (int v = 0; v < N; ++v) if (!(std::cin >> x[v] >> y[v])) return 1;
    std::vector<Edge> edges;
    for (int i = 0; i < M; ++i) {
        int u, v;
        if (!(std::cin >> u >> v)) return 1;
        const int dx = x[u] - x[v], dy = y[u] - y[v];
        edges.push_back({u, v, static_cast<int>(std::lround(std::sqrt(dx*dx+dy*dy))), i});
    }
#ifndef AHC_MEAN_WEIGHTS
#ifdef AHC_SAMPLE_COUNT
    constexpr int scenarios = AHC_SAMPLE_COUNT;
#else
    constexpr int scenarios = 24;
#endif
#else
    constexpr int scenarios = 1;
#endif
    std::array<std::vector<Edge>, scenarios> orders;
#ifdef AHC_PIVOTAL_CROSSFIT
    std::array<std::vector<Edge>, AHC_PIVOTAL_CROSSFIT> independent_backup;
#endif
#ifdef AHC_REGRET
    // The nominal future without the actual accepted prefix.
    auto distance_order=edges;
    std::sort(distance_order.begin(),distance_order.end(),[](auto a,auto b){return a.cost<b.cost;});
    std::array<int,M> reserve;
    for(const auto& e:edges) {
        Dsu future(N);
        const auto cap=kruskal_path_bottleneck(future,distance_order,e.from,e.to,
            [&](const auto& other){return other.id>e.id;});
#ifndef AHC_REGRET_CAP
#define AHC_REGRET_CAP 200
#endif
        reserve[e.id]=cap ? (*cap*AHC_REGRET_CAP+50)/100 : 4096;
    }
#endif

#ifndef AHC_MEAN_WEIGHTS
    std::mt19937 rng(271907);
#endif
#ifdef AHC_ANTITHETIC
    static_assert(scenarios%2==0);
    for(auto& order:orders) order=edges;
    for(int k=0;k<scenarios;k+=2) {
        for(const auto& e:edges) {
            int w=std::uniform_int_distribution<int>(e.cost,3*e.cost)(rng);
            orders[k][e.id].cost=w;
            orders[k+1][e.id].cost=4*e.cost-w;
        }
    }
#elif defined(AHC_NESTED)
    static_assert((scenarios & (scenarios-1))==0);
    for(auto& order:orders) order=edges;
    for(const auto& e:edges) {
        // Scramble each dimension separately. For every prefix of 2^p
        // scenarios, each of the 2^p dyadic bins gets one assignment.
        std::array<std::uint8_t,scenarios> flip;
        for(int z=1;z<scenarios;++z) flip[z]=rng()&1;
        const int span=2*e.cost+1;
        for(int k=0;k<scenarios;++k) {
            int code=0, node=1;
            for(int v=k;node<scenarios;v>>=1) {
                int b=v&1;
                code=2*code+(b^flip[node]);
                node=2*node+b;
            }
            std::uint64_t q=(std::uint64_t(code)<<32)+rng();
            orders[k][e.id].cost=e.cost+int(q*span/(std::uint64_t(scenarios)<<32));
        }
    }
#elif defined(AHC_STRATIFIED)
    for(auto& order:orders) order=edges;
    std::array<int,scenarios> perm;
    std::iota(perm.begin(),perm.end(),0);
    for(const auto& e:edges) {
        std::shuffle(perm.begin(),perm.end(),rng);
        int span=2*e.cost+1;
        for(int k=0;k<scenarios;++k) {
            const std::uint64_t q=(std::uint64_t(perm[k])<<32) + rng();
            orders[k][e.id].cost=e.cost+int(q*span/(std::uint64_t(scenarios)<<32));
        }
    }
#endif
    for (auto& order : orders) {
#if !defined(AHC_STRATIFIED) && !defined(AHC_ANTITHETIC) && !defined(AHC_NESTED)
        order = edges;
        for (auto& e : order) {
#ifndef AHC_MEAN_WEIGHTS
            e.cost = std::uniform_int_distribution<int>(e.cost, 3*e.cost)(rng);
#else
            e.cost *= 2;
#endif
        }
#endif
#ifdef AHC_REGRET
        for(auto& e:order) {
            int gap=std::max(0,e.cost-reserve[e.id]);
            e.cost+=(gap*AHC_REGRET+50)/100;
        }
#endif
        std::sort(order.begin(), order.end(), [](const Edge& a, const Edge& b) {
            if (a.cost != b.cost) return a.cost < b.cost;
            return a.id < b.id;
        });
    }
#ifdef AHC_PIVOTAL_CROSSFIT
    for(auto& o:independent_backup) {
        o=edges;
        for(auto& e:o) e.cost=std::uniform_int_distribution<int>(e.cost,3*e.cost)(rng);
        std::sort(o.begin(),o.end(),[](const Edge& a,const Edge& b) {
            return std::pair(a.cost,a.id)<std::pair(b.cost,b.id);
        });
    }
#endif
#ifdef AHC_PAIRED_ROLLOUT
    auto mean_order = edges;
    for (auto& e : mean_order) e.cost *= 2;
    std::sort(mean_order.begin(), mean_order.end(), [](const auto& a, const auto& b) {
        return a.cost != b.cost ? a.cost < b.cost : a.id < b.id;
    });
    std::array<std::array<int, M>, scenarios> weights;
    for (int k = 0; k < scenarios; ++k)
        for (const auto& e : orders[k]) weights[k][e.id] = e.cost;
#endif
#ifdef AHC_FAST_SAMPLES
#ifndef AHC_BLOCK
#define AHC_BLOCK 128
#endif
    using Cert=SuffixCertificates<N,M,AHC_BLOCK,Edge>;
    std::vector<Cert> certs;
    certs.reserve(scenarios);
    for(const auto& order:orders) certs.emplace_back(order);
#endif
#ifdef AHC_BIDIR_SAMPLES
    BidirBottleneck<N,M> bidir(edges);
    std::vector<std::array<std::uint16_t,M>> bidir_weights(scenarios);
    for(int k=0;k<scenarios;++k)
        for(const auto& e:orders[k]) bidir_weights[k][e.id]=e.cost;
    bool groups_dirty=true;
#endif
#ifdef AHC_ADAPTIVE
    auto mean_adaptive=edges;
    for(auto& e:mean_adaptive) e.cost*=2;
    std::sort(mean_adaptive.begin(),mean_adaptive.end(),[](const auto& a,const auto& b){return a.cost<b.cost;});
#endif
#ifdef AHC_PIVOTAL
    int max_weight=0;
    for(auto e:edges) max_weight=std::max(max_weight,3*e.cost);
#endif
#ifdef AHC_ACTIVE_SAMPLES
    ActiveKruskal<N,M> active(edges,orders);
#endif
    Dsu tree(N), scratch(N);
    auto old_cutoff=[&](int i, const Edge& e, const std::vector<Edge>& order) {
        scratch = tree;
        return kruskal_path_bottleneck(scratch, order, e.from, e.to,
            [i](const Edge& other){return other.id>i;});
    };
    for (int i = 0; i < M; ++i) {
        int length;
        if (!(std::cin >> length)) return 1;
        const auto& e = edges[i];
#ifdef AHC_ACTIVE_SAMPLES
        active.erase(i);
#endif
        bool take = false;
        if (!tree.same(e.from, e.to)) {
            // Connectivity without e is independent of scenario costs.
            int total_cutoff = 0, count = 0;
#ifdef AHC_PAIRED_ROLLOUT
            std::array<int, scenarios> cutoffs{};
            if (std::chrono::steady_clock::now() >= scenario_deadline) {
                scratch = tree;
                const auto cutoff = kruskal_path_bottleneck(scratch, mean_order, e.from, e.to,
                    [i](const Edge& other) { return other.id > i; });
                take = !cutoff || length <= *cutoff;
            } else {
#endif
#if defined(AHC_ADAPTIVE)
            ++query_count;
            std::int64_t sq=0;
#if defined(AHC_PIVOTAL) || defined(AHC_RISK)
            std::array<int,scenarios> ids{};
#if defined(AHC_PIVOTAL_RANK) || defined(AHC_SIGNED_RESIDUAL)
            std::array<int,scenarios> limits{};
#endif
#endif
#ifdef AHC_ANTITHETIC
            int previous=0;
#endif
            if(std::chrono::steady_clock::now()>=sample_deadline) {
                ++fallback_count;
                auto limit=old_cutoff(i,e,mean_adaptive);
                take=!limit || length<=*limit;
            } else {
                for(int k=0; k<scenarios; ++k) {
                    const auto& order=orders[k];
#ifdef AHC_ACTIVE_SAMPLES
#if defined(AHC_PIVOTAL) || defined(AHC_RISK)
                    const int packed=active.simulations[k].template cutoff<true>(active.parent,e.from,e.to);
                    const int w=packed<0?-1:(packed>>16);
                    ids[count]=packed&65535;
#else
                    const int w=active.simulations[k].template cutoff<false>(active.parent,e.from,e.to);
#endif
                    auto limit=w<0 ? std::optional<int>{} : std::optional<int>{w};
#elif defined(AHC_PIVOTAL) || defined(AHC_RISK)
                    scratch=tree;
                    int bid=-1,weight=0;
                    for(const auto& f:order) {
                        if(f.id<=i || !scratch.unite(f.from,f.to)) continue;
                        if(scratch.same(e.from,e.to)) {bid=f.id;weight=f.cost;break;}
                    }
                    auto limit=bid<0 ? std::optional<int>{} : std::optional<int>{weight};
                    ids[count]=bid;
#else
                    auto limit=old_cutoff(i,e,order);
#endif
                    if(!limit) {take=true;break;}
#if defined(AHC_PIVOTAL_RANK) || defined(AHC_SIGNED_RESIDUAL)
                    limits[count]=*limit;
#endif
                    ++count;
                    total_cutoff+=*limit;
#ifdef AHC_ANTITHETIC
                    if(!(count&1)) sq+=(*limit+previous)*(*limit+previous);
                    previous=*limit;
#else
                    sq+=*limit * *limit;
#endif
                    if(count==8 || count==16 || count==32 || count==64
#ifdef AHC_DEEP_ADAPT
                       || count==128 || count==256
#endif
                       ) {
                        // Conservative early exit away from the near-tie region.
                        const auto gap=std::int64_t(length)*count-total_cutoff;
#ifdef AHC_ANTITHETIC
                        const int independent=count/2;
#else
                        const int independent=count;
#endif
                        const auto var=sq*independent-std::int64_t(total_cutoff)*total_cutoff;
                        constexpr int Z100=AHC_ADAPTIVE;
                        if(std::abs(gap)>2*count &&
                           (std::abs(gap)-2*count)*(std::abs(gap)-2*count)*10000*(independent-1) >
                               std::int64_t(Z100)*Z100*var) {
                            ++short_count;
                            break;
                        }
                    }
                }
                sample_count+=count;
                take=take || count*length<=total_cutoff;
#if defined(AHC_PIVOTAL) || defined(AHC_RISK)
                const int gap=count*length-total_cutoff;
                if(count) {
#ifndef AHC_PIVOTAL_BAND
#define AHC_PIVOTAL_BAND 10
#endif
#ifdef AHC_PIVOTAL
                    constexpr int K=AHC_PIVOTAL;
#else
                    constexpr int K=2;
#endif
                    // Risk is relevant for near decisions only. Far from the
                    // threshold, cap its influence to the same screening band.
                    if(std::abs(gap)<=AHC_PIVOTAL_BAND*count) {
                        auto freq=ids;
                        std::sort(freq.begin(),freq.begin()+count);
                        struct Rank {
                            double merit=0;
                            int id=-1,occ=0;
                        };
                        std::array<Rank,K> ranks{};
                        int collisions=0, geometric_sum=0;
                        for(int l=0;l<count;) {
                            int r=l+1;
                            while(r<count&&freq[r]==freq[l])++r;
                            collisions+=(r-l)*(r-l-1);
                            int id=freq[l];
                            geometric_sum+=(r-l)*edges[id].cost;
                            double merit=r-l;
#ifdef AHC_PIVOTAL_RANK
                            if(r-l>=2 && count>=24) {
                                double cx=0,cxx=0,cxy=0;
                                for(int k=0;k<count;++k) {
                                    const auto& sim=active.simulations[k];
                                    int w=sim.sorted[sim.pos[id]].w;
                                    cx+=w; cxx+=w*w; cxy+=w*limits[k];
                                }
                                const double var=cxx-cx*cx/count;
                                const double cov=cxy-cx*total_cutoff/count;
                                if(var>0 && cov>0)
                                    merit+=AHC_PIVOTAL_RANK/100.0*cov/std::sqrt(var*count);
                            }
#endif
                            Rank p{merit,id,r-l};
                            for(auto& b:ranks)
                                if(p.merit>b.merit || (p.merit==b.merit&&p.id<b.id))
                                    std::swap(p,b);
                            l=r;
                        }
                        double correction=0;
                        int coverage=0;
                        int residual_sum=total_cutoff;
#ifdef AHC_PIVOTAL
                        if(count>=24 && ranks[K-1].occ>0 && ranks[1].occ>=2 &&
                           std::chrono::steady_clock::now()<rollout_deadline) {
                            ++pivotal_count;
                            std::array<int,K> picked{};
                            int picked_count=0;
#ifdef AHC_FOCUS_CUT
                            int best_support=-1;
                            for(int side:{e.from,e.to}) {
                                std::array<int,K> alternative;
                                const int n=active.template boundary<K>(side,i,alternative);
                                if(n<2 || n>K) continue;
                                int support=0;
                                for(int j=0;j<n;++j) {
                                    auto [l,r]=std::equal_range(freq.begin(),freq.begin()+count,alternative[j]);
                                    support+=r-l;
                                }
                                if(support>best_support && support*100>=AHC_FOCUS_CUT*count) {
                                    best_support=support;
                                    picked_count=n;
                                    for(int j=0;j<n;++j)picked[j]=alternative[j];
                                }
                            }
                            focused+=picked_count>0;
#endif
                            for(int j=0;j<K && picked_count<K;++j) {
                                int edge=ranks[j].id;
                                auto end=picked.begin()+picked_count;
                                if(std::find(picked.begin(),end,edge)==end)
                                    picked[picked_count++]=edge;
                            }
                            assert(picked_count==K);

                            for(int j=0;j<K;++j) {
                                auto [lo,hi]=std::equal_range(freq.begin(),freq.begin()+count,picked[j]);
                                int occurred=hi-lo;
                                coverage+=occurred;
#ifdef AHC_SIGNED_RESIDUAL
                                geometric_sum-=occurred*edges[picked[j]].cost;
                                for(int s=0;s<count;++s) if(ids[s]==picked[j]) residual_sum-=limits[s];
#endif
                            }
                            PivotalRollout<K,N,M> rollout(edges,i,picked,max_weight);
#ifndef AHC_PIVOTAL_MC
#define AHC_PIVOTAL_MC 12
#endif
                            constexpr int replay=AHC_PIVOTAL_MC;
                            double offline_sampled=0;
                            for(int r=0;r<replay;++r) {
#ifdef AHC_PIVOTAL_CROSSFIT
                                static_assert(replay<=AHC_PIVOTAL_CROSSFIT);
                                const auto [cv_online,cv_offline]=rollout.conditional_values(tree,
                                    independent_backup[(i+r)%AHC_PIVOTAL_CROSSFIT],i);
                                correction+=cv_online-cv_offline;
                                offline_sampled+=cv_offline;
#else
                                int k=(i+(r*scenarios)/replay)%scenarios;
                                const auto [cv_online,cv_offline]=rollout.conditional_values(tree,orders[k],i);
                                correction+=cv_online-cv_offline;
                                offline_sampled+=cv_offline;
#endif
                            }
#ifndef AHC_PIVOTAL_POWER
#define AHC_PIVOTAL_POWER 100
#endif
                            correction*=AHC_PIVOTAL_POWER/(100.0*replay);
#ifdef AHC_RAO_BLEND
                            correction+=AHC_RAO_BLEND/100.0*(offline_sampled/replay-double(total_cutoff)/count);
#endif
                            pivotal_total_adjust+=int(std::lround(correction*100));
                        }
#endif
#ifdef AHC_RISK
                        if(count>1) {
                            double ratio=double(collisions)/(count*(count-1));
                            double sd=std::sqrt(double(sq*count-std::int64_t(total_cutoff)*total_cutoff)/(count*(count-1)));
                            double covered=double(coverage)/count;
                            double c=AHC_RISK/100.0;
#ifdef AHC_RISK_TIME
                            c*=(1+AHC_RISK_TIME/100.0*(0.5-double(i)/M));
#endif
#ifdef AHC_SIGNED_RISK
                            const double slack=geometric_sum?2.0-double(residual_sum)/geometric_sum:0.0;
#ifndef AHC_SIGNED_OFFSET
#define AHC_SIGNED_OFFSET 0
#endif
                            c*=std::clamp(AHC_SIGNED_OFFSET/100.0+AHC_SIGNED_RISK/100.0*slack,-1.75,2.2);
#endif
                            correction+=c*sd*(1-ratio)*(1-covered*covered);
                        }
#endif
                        bool revised=gap<=count*correction;
#ifdef AHC_PIVOTAL
                        pivotal_changed+=revised!=take;
#endif
                        take=revised;
                    }
                }
#ifdef AHC_SMALLBLOCK
#ifndef AHC_SMALLBLOCK_START
#define AHC_SMALLBLOCK_START 900
#endif
                if(count>=8 && i>=AHC_SMALLBLOCK_START &&
                   std::abs(gap)<=12*count &&
                   std::chrono::steady_clock::now()<sample_deadline) {
                    ++blocks_considered;
                    const auto exact=VertexBlock<N,M>::limit<AHC_SMALLBLOCK>(tree,edges,i);
                    if(exact) {
                        ++blocks_exact;
                        const bool best=length<=*exact;
                        blocks_changed+=best!=take;
                        take=best;
                    }
                }
#endif
#endif
            }
#elif defined(AHC_ACTIVE_SAMPLES)
            for(const auto& sim:active.simulations) {
                int c=sim.template cutoff<false>(active.parent,e.from,e.to);
                if(c<0){take=true;break;}
                ++count;
                total_cutoff+=c;
            }
#elif defined(AHC_BIDIR_SAMPLES)
            if(groups_dirty) {
                bidir.set_groups(tree);
                groups_dirty=false;
            }
            for(const auto& w:bidir_weights) {
                int cutoff=bidir.cutoff(w,i,e.from,e.to);
                if(cutoff<0) {take=true;break;}
                ++count;
                total_cutoff+=cutoff;
            }
#elif defined(AHC_FAST_SAMPLES)
            Cert::Parents parents;
            for(int v=0;v<N;++v) parents[v]=tree.parent_or_size[v];
            for(const auto& cert:certs) {
                int cutoff=cert.cutoff(parents,i,e.from,e.to);
                if(cutoff<0) {take=true;break;}
                ++count;
                total_cutoff+=cutoff;
            }
#else
            for (const auto& order : orders) {
                scratch = tree;
                auto cutoff = kruskal_path_bottleneck(scratch, order, e.from, e.to,
                    [i](const Edge& other) { return other.id > i; });
                if (!cutoff) { take = true; break; }
#ifdef AHC_PAIRED_ROLLOUT
                cutoffs[count] = *cutoff;
#endif
                ++count;
                total_cutoff += *cutoff;
            }
#endif
#ifndef AHC_ADAPTIVE
            take = take || scenarios * length <= total_cutoff;
#endif
#ifdef AHC_PAIRED_ROLLOUT
            // Only resolve near ties. Compute the excess online cost gap
            // relative to the exact per-scenario offline cut, then add this
            // paired control variate to the estimate from all scenarios.
            if (count == scenarios && rollouts < 160 && i + 5 < M &&
                12 * std::abs(scenarios * length - total_cutoff) <= total_cutoff &&
                std::chrono::steady_clock::now() < rollout_deadline) {
                ++rollouts;
                Dsu accepted = tree;
                accepted.unite(e.from, e.to);
                constexpr int paired = 6, horizon = 16;
                int correction = 0, sampled = 0;
                const int end = std::min(M, i + 1 + horizon);
                for (int s = 0; s < paired; ++s) {
                    if (std::chrono::steady_clock::now() >= rollout_deadline) break;
                    const int k = (i + s * 4) % scenarios;
                    int yes = play_then_complete(accepted, i+1, end, edges, mean_order, weights[k], orders[k]);
                    int no = play_then_complete(tree, i+1, end, edges, mean_order, weights[k], orders[k]);
#ifdef AHC_MATCHED_REPLACEMENT
                    const int short_end=end-1;
                    const auto pre_yes=play_prefix(accepted,i+1,short_end,edges,mean_order,weights[k]);
                    const auto pre_no=play_prefix(tree,i+1,short_end,edges,mean_order,weights[k]);
                    const auto suffix_no=suffix(pre_no,short_end,orders[k]);
                    int release=suffix_no.cost<0 ? -1 : final_release(pre_no.tree,suffix_no.selected,e.from,e.to);
                    if(release>=end) {
                        const auto suffix_yes=suffix(pre_yes,short_end,orders[k]);
                        if(suffix_yes.cost>=0) {
                            const int new_yes=can_accept_at_release(pre_yes,suffix_yes,release,mean_order,orders[k]);
                            const int new_no=can_accept_at_release(pre_no,suffix_no,release,mean_order,orders[k]);
                            ++targeted;
                            late_changes+=(new_yes!=suffix_yes.cost || new_no!=suffix_no.cost);
#ifdef AHC_USE_REPLACEMENT
                            yes=new_yes;
                            no=new_no;
#endif
                        }
                    }
#endif
                    if (yes < 0 || no < 0) break;
                    correction += yes - no + cutoffs[k];
                    ++sampled;
                }
                if (sampled == paired) {
                    const bool revised =
                        paired * (scenarios * length - total_cutoff) + scenarios * correction <= 0;
                    changes += take != revised;
                    take = revised;
                }
            }
            }  // scenario work pays attention to the wall clock
#endif
            if (take) {
                tree.unite(e.from, e.to);
#ifdef AHC_ACTIVE_SAMPLES
                active.merge(e.from,e.to,i);
#endif
#ifdef AHC_BIDIR_SAMPLES
                groups_dirty=true;
#endif
            }
        }
        std::cout << take << '\n' << std::flush;
    }
#ifdef AHC_PAIRED_ROLLOUT
    std::cerr << "paired_probes=" << rollouts << " revised_actions=" << changes;
#ifdef AHC_MATCHED_REPLACEMENT
    std::cerr << " late_checked=" << targeted << " late_rejections=" << late_changes;
#endif
    std::cerr << '\n';
#endif
#ifdef AHC_ADAPTIVE
    std::cerr<<"queries="<<query_count<<" samples="<<sample_count
             <<" short="<<short_count<<" fallback="<<fallback_count;
#ifdef AHC_PIVOTAL
    std::cerr<<" pivotal="<<pivotal_count<<" changed="<<pivotal_changed;
#ifdef AHC_FOCUS_CUT
    std::cerr<<" focused="<<focused;
#endif
#endif
#ifdef AHC_SMALLBLOCK
    std::cerr<<" block_probes="<<blocks_considered<<" exact_block="<<blocks_exact
             <<" block_revised="<<blocks_changed;
#endif
    std::cerr<<'\n';
#endif
    return tree.component_count() == 1 ? 0 : 1;
}
