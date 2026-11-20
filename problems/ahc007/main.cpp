#include <algorithm>
#include <array>
#include <cmath>
#ifdef AHC_PAIRED_ROLLOUT
#include <chrono>
#endif
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include "../../library/dsu.hpp"
#include "../../library/kruskal-bottleneck.hpp"

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
    constexpr int scenarios = 24;
#else
    constexpr int scenarios = 1;
#endif
    std::array<std::vector<Edge>, scenarios> orders;
#ifndef AHC_MEAN_WEIGHTS
    std::mt19937 rng(271907);
#endif
    for (auto& order : orders) {
        order = edges;
        for (auto& e : order) {
#ifndef AHC_MEAN_WEIGHTS
            e.cost = std::uniform_int_distribution<int>(e.cost, 3*e.cost)(rng);
#else
            e.cost *= 2;
#endif
        }
        std::sort(order.begin(), order.end(), [](const Edge& a, const Edge& b) {
            if (a.cost != b.cost) return a.cost < b.cost;
            return a.id < b.id;
        });
    }
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
    Dsu tree(N), scratch(N);
    for (int i = 0; i < M; ++i) {
        int length;
        if (!(std::cin >> length)) return 1;
        const auto& e = edges[i];
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
            take = take || scenarios * length <= total_cutoff;
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
            if (take) tree.unite(e.from, e.to);
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
    return tree.component_count() == 1 ? 0 : 1;
}
