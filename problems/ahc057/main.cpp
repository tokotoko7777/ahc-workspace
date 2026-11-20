#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <cmath>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>
#include "../../library/periodic-closest-approach.hpp"

struct Point {
    double x{},y{};
    Point operator+(Point a) const {return {x+a.x,y+a.y};}
    Point operator-(Point a) const {return {x-a.x,y-a.y};}
    Point operator*(double s) const {return {x*s,y*s};}
};
struct Plan {
    int cost{};
    std::vector<std::tuple<int,int,int>> edges;
    void add(const Plan& p) {
        cost+=p.cost;
        edges.insert(edges.end(),p.edges.begin(),p.edges.end());
    }
};
struct Problem {
    int n,T,m,k,L;
    std::vector<Point> p,v;
    double norm2(Point a) const {
        const double x=std::remainder(a.x,L),y=std::remainder(a.y,L);
        return x*x+y*y;
    }
    int edge(Point a,Point b) const { return std::lround(std::sqrt(norm2(a-b))); }
    std::vector<std::vector<int>> partition(const std::vector<Point>& pts,int first) const {
        std::vector<Point> center{pts[first]};
        std::vector<double> near(n,std::numeric_limits<double>::infinity());
        while(int(center.size())<m) {
            for(int i=0;i<n;++i) near[i]=std::min(near[i],norm2(pts[i]-center.back()));
            center.push_back(pts[std::max_element(near.begin(),near.end())-near.begin()]);
        }
        std::vector<std::vector<int>> group(m);
        for(int repeat=0;repeat<5;++repeat) {
            for(auto& g:group) g.clear();
            std::vector<int> used(n);
            std::vector<std::vector<double>> dist(n,std::vector<double>(m));
            for(int i=0;i<n;++i) for(int j=0;j<m;++j) dist[i][j]=norm2(pts[i]-center[j]);
            for(int pos=0;pos<n;++pos) {
                double max_gap=-1;
                int pick=-1,gid=-1;
                for(int i=0;i<n;++i) if(!used[i]) {
                    double d1=1e30,d2=1e30;
                    int id=-1;
                    for(int j=0;j<m;++j) if(int(group[j].size())<k) {
                        double d=dist[i][j];
                        if(d<d1) d2=d1,d1=d,id=j;
                        else if(d<d2) d2=d;
                    }
                    if(d2-d1>max_gap) max_gap=d2-d1,pick=i,gid=id;
                }
                used[pick]=true;group[gid].push_back(pick);
            }
            for(int j=0;j<m;++j) {
                Point shift{};
                for(int i:group[j]) {
                    Point d=pts[i]-center[j];
                    shift=shift+Point{std::remainder(d.x,L),std::remainder(d.y,L)};
                }
                center[j]=center[j]+shift*(1.0/k);
            }
        }
        return group;
    }
    Plan mst(const std::vector<int>& g,const std::vector<Point>& pts,int t) const {
        Plan out;
        std::vector<int> dist(k,1<<30),pre(k,-1),used(k);
        dist[0]=0;
        for(int s=0;s<k;++s) {
            int pick=-1;
            for(int i=0;i<k;++i) if(!used[i] && (pick<0 || dist[i]<dist[pick])) pick=i;
            used[pick]=true;
            if(pre[pick]>=0) out.cost+=dist[pick],out.edges.emplace_back(t,g[pre[pick]],g[pick]);
            for(int j=0;j<k;++j) if(!used[j]) {
                int d=edge(pts[g[pick]],pts[g[j]]);
                if(d<dist[j]) dist[j]=d,pre[j]=pick;
            }
        }
        return out;
    }
    Plan moving(const std::vector<int>& g,double wait_cost) const {
        std::vector<Point> points(k),velocity(k);
        std::vector<std::vector<int>> connected(k);
        std::vector<int> active(k);
        std::iota(active.begin(),active.end(),0);
        for(int i=0;i<k;++i) points[i]=p[g[i]],velocity[i]=v[g[i]],connected[i]={i};
        Plan out;
        int now=0;
        while(active.size()>1) {
            double value=std::numeric_limits<double>::infinity();
            int a=-1,b=-1,i=-1,j=-1,t=0,cost=0;
            // Recheck at increasing horizons: distant opportunities include more deferral.
            // After committing, all later costs are computed from the new component velocities.
            for(unsigned u=0;u<active.size();++u) for(unsigned w=u+1;w<active.size();++w) {
                const int A=active[u],B=active[w];
                const Point dv=velocity[A]-velocity[B];
                for(int I:connected[A]) for(int J:connected[B]) {
                    const Point dx=points[I]-points[J];
                    for(int horizon:{70,230,T}) {
                        auto z=periodic_closest_approach(dx.x,dx.y,dv.x,dv.y,L,L,0,std::min(T-1-now,horizon));
                        double d=std::sqrt(z.squared_distance),r=d+wait_cost*z.time;
                        if(r<value) value=r,a=A,b=B,i=I,j=J,t=z.time,cost=std::lround(d);
                    }
                }
            }
            out.edges.emplace_back(now+t,g[i],g[j]);out.cost+=cost;
            for(int r:active) for(int u:connected[r]) points[u]=points[u]+velocity[r]*t;
            now+=t;
            const double sa=connected[a].size(),sb=connected[b].size();
            velocity[a]=(velocity[a]*sa+velocity[b]*sb)*(1/(sa+sb));
            connected[a].insert(connected[a].end(),connected[b].begin(),connected[b].end());
            connected[b].clear();
            active.erase(std::find(active.begin(),active.end(),b));
        }
        return out;
    }
    Plan plan_group(const std::vector<int>& g,const std::vector<Point>& pts,int t) const {
        Plan pick=mst(g,pts,t);
#ifndef AHC057_STATIC
        for(double delay : {0.,6.,20.}) {
            Plan trial=moving(g,delay);
            if(trial.cost<pick.cost) pick=std::move(trial);
        }
#endif
        return pick;
    }
    // Two variants use the same sample/attempt budget and exact evaluator.
    void exchange(std::vector<std::vector<int>>& groups,std::vector<Plan>& plans,
                  const std::vector<Point>& at,int time,bool periodic,
                  std::chrono::steady_clock::time_point deadline) const {
        std::vector<std::vector<double>> d(n,std::vector<double>(n));
        for(int i=0;i<n;++i) for(int j=0;j<i;++j) {
            Point diff=p[i]-p[j],vel=v[i]-v[j];
            d[i][j]=d[j][i]=periodic ? std::sqrt(periodic_closest_approach(
                diff.x,diff.y,vel.x,vel.y,L,L,std::max(0,time-350),std::min(T-1,time+350)).squared_distance)
                : std::sqrt(norm2(at[i]-at[j]));
        }
        uint32_t state=1845116969;
        auto rng=[&]() {
            state^=state<<13;state^=state>>17;state^=state<<5;
            return state;
        };
        auto fit=[&](int u,const std::vector<int>& group,int skip) {
            std::array<double,3> best{1e20,1e20,1e20};
            for(int j:group) if(j!=skip) {
                double value=d[u][j];
                for(double& prior:best) if(value<prior) std::swap(value,prior);
            }
            return best[0]+best[1]+best[2];
        };
        for(int iteration=0;iteration<32 && std::chrono::steady_clock::now()<deadline;++iteration) {
            int out_a=0,out_b=0,out_x=0,out_y=0;
            double value=-1e30;
            for(int draw=0;draw<72;++draw) {
                int a=rng()%m,b=(a+1+rng()%(m-1))%m,x=rng()%k,y=rng()%k;
                int u=groups[a][x],v=groups[b][y];
                double gain=fit(u,groups[a],u)+fit(v,groups[b],v)
                           -fit(u,groups[b],v)-fit(v,groups[a],u);
                if(gain>value) value=gain,out_a=a,out_b=b,out_x=x,out_y=y;
            }
            auto& a=groups[out_a];auto& b=groups[out_b];
            std::swap(a[out_x],b[out_y]);
            Plan aa=plan_group(a,at,time),bb=plan_group(b,at,time);
            if(aa.cost+bb.cost < plans[out_a].cost+plans[out_b].cost) {
                plans[out_a]=std::move(aa);plans[out_b]=std::move(bb);
            } else std::swap(a[out_x],b[out_y]);
        }
    }

};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    [[maybe_unused]] const auto stop=std::chrono::steady_clock::now()+std::chrono::milliseconds(1550);
    Problem q;
    if(!(std::cin>>q.n>>q.T>>q.m>>q.k>>q.L)) return 0;
    q.p.resize(q.n);q.v.resize(q.n);
    for(int i=0;i<q.n;++i) std::cin>>q.p[i].x>>q.p[i].y>>q.v[i].x>>q.v[i].y;
    std::vector<std::vector<int>> best_groups;
    std::vector<Plan> best_static;
    int best=std::numeric_limits<int>::max(),best_time=0;
    std::vector<Point> best_points;
    for(int s=0;s<=10;++s) {
        const int t=s*(q.T-1)/10;
        std::vector<Point> pts(q.n);
        for(int i=0;i<q.n;++i) pts[i]=q.p[i]+q.v[i]*t;
        auto groups=q.partition(pts,(79*s)%q.n);
        std::vector<Plan> plans;
        int cost=0;
        for(auto& g:groups) {
            plans.push_back(q.mst(g,pts,t));
            cost+=plans.back().cost;
        }
        if(cost<best) best=cost,best_time=t,best_points=std::move(pts),
            best_groups=std::move(groups),best_static=std::move(plans);
    }
    for(int i=0;i<q.m;++i) best_static[i]=q.plan_group(best_groups[i],best_points,best_time);
#if defined(AHC057_SWAP_PERIODIC)
    q.exchange(best_groups,best_static,best_points,best_time,true,stop);
#elif !defined(AHC057_STATIC) && !defined(AHC057_NO_EXCHANGE)
    q.exchange(best_groups,best_static,best_points,best_time,false,stop);
#endif
    Plan answer;
    for(const auto& plan:best_static) answer.add(plan);
    std::sort(answer.edges.begin(),answer.edges.end());
    for(auto [t,a,b]:answer.edges) std::cout<<t<<' '<<a<<' '<<b<<'\n';
    std::cerr<<"Cost = "<<answer.cost<<'\n';
}
