#pragma GCC optimize("O3")
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

using Money=long long;
struct Project { Money work,value; };
struct Card { int type; Money work,price=0; };

struct Policy {
    int level=0;
    double market_rate=18.;
    std::vector<Project> p;
#if defined(AHC029_TURNOVER) || defined(AHC029_GROWTH_RESERVE)
    std::vector<int> birth;
#endif
    long long unit() const {return 1LL<<level;}

    void observe(const std::vector<Card>& offers) {
        double top=1;
        for(const auto& c:offers) if(c.type==0||c.type==1) {
            const double work=double(c.work)/unit();
            const double price=double(c.price)/unit()/(c.type==1?p.size():1.);
            if(price<=1.18*work) top=std::max(top,work*(c.type==1?1.45:1.));
        }
        market_rate=.955*market_rate+.045*top;
    }
    double utility(const Card& c,int target,int remaining,Money money) const {
        const double scale=unit();
        if(c.type==4) return level<20?1e30:-1e30;
#ifndef AHC029_MARKET_FORECAST
        (void)money;
        auto abandon=[&](const Project& q) {
            if(remaining<4) return -1e20;
#if defined(AHC029_TURNOVER) || defined(AHC029_GROWTH_RESERVE)
            if(remaining>=30 && birth[&q-p.data()]<level) {
                // Old projects with short remaining work are still cheap to finish.
                const double h=q.work/scale,v=q.value/scale;
                if(h>=9 && v<=h+7)
                    return scale*(9.+std::min(18.,.34*h)-.26*v);
            }
#endif
            return q.work>80*scale && q.value<.91*q.work?10.*scale:-8.*scale;
        };
        auto work=[&](const Project& q) {
            if(q.work<=c.work) return double(q.value);
            if(remaining==1) return 0.;
            return double(q.value)*c.work/q.work;
        };
#else
        const double tau=std::max(3.,std::min(remaining,4)*std::max(8.,market_rate))*scale;
        auto potential=[&](Money h,Money v){return v*std::exp(-h/tau);};
        double replacement=0.;
        if(remaining>5) {
            for(double h:{5.,9.,18.,36.,72.,144.,216.})
                replacement+=potential(h*scale,h*scale*1.067);
            replacement/=7.;
        }
        auto work=[&](const Project& q) {
            if(q.work<=c.work)
                return q.value*(1.+(money<32*scale?.14:0.))+.30*replacement;
            if(remaining==1) return 0.;
            const double fraction=double(c.work)/q.work;
            const double marginal=potential(q.work-c.work,q.value)-potential(q.work,q.value);
            const double terminal=remaining<9 ? std::min(1.,remaining*market_rate*scale/q.work) : 1.;
            return (.44*q.value*fraction+1.55*marginal)*terminal;
        };
        auto abandon=[&](const Project& q) {
            if(remaining<5) return -1e20;
            // Value an empty slot only against the known market throughput,
            // and avoid using scarce turns to replace a nearly ready job.
            double lost=potential(q.work,q.value);
            return .66*(replacement-lost)-scale*(3.+.13*market_rate);
        };
#endif
        if(c.type==0) return work(p[target]);
        if(c.type==2) return abandon(p[target]);
        double total=0.;
        for(const auto& q:p) total+=c.type==1?work(q):abandon(q);
        return total;
    }

    std::pair<double,int> target_for(const Card& c,int remaining,Money money) const {
        int where=0;
        double best=utility(c,0,remaining,money);
        if(c.type==0||c.type==2) for(int j=1;j<int(p.size());++j) {
            auto u=utility(c,j,remaining,money);
            if(u>best) best=u,where=j;
        }
        return {best,where};
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    int n,m,k,t;
    if(!(std::cin>>n>>m>>k>>t)) return 1;
    Policy policy;
    std::vector<Card> hand(n),offer(k);
    policy.p.resize(m);
#if defined(AHC029_TURNOVER) || defined(AHC029_GROWTH_RESERVE)
    policy.birth.resize(m);
#endif
    for(auto& x:hand) std::cin>>x.type>>x.work;
    for(auto& p:policy.p) std::cin>>p.work>>p.value;
    Money money=0;
    for(int turn=0;turn<t;++turn) {
        const int rem=t-turn;
        int c=0,j=0;
        double best=-1e100;
        for(int i=0;i<n;++i) {
            auto [value,where]=policy.target_for(hand[i],rem,money);
#if defined(AHC029_TURNOVER) || defined(AHC029_GROWTH_RESERVE)
            if(policy.level && rem>30 && hand[i].type<2 && hand[i].work*4<policy.unit()*10)
                value+=4.*policy.unit(); // recycle weak labor, unless a stronger payoff wins
#endif
            if(value>best) c=i,j=where,best=value;
        }
        if(hand[c].type==4) {
            if(policy.level>=20) return 1;
            ++policy.level;
        }
#if defined(AHC029_TURNOVER) || defined(AHC029_GROWTH_RESERVE)
        const Card used=hand[c];
        for(int i=0;i<m;++i) {
            const bool touched=used.type==1||used.type==3||i==j;
            const bool replaced=((used.type==2||used.type==3)||
                                 (used.type<2&&policy.p[i].work<=used.work));
            if(touched && replaced) policy.birth[i]=policy.level;
        }
#endif
        std::cout<<c<<' '<<j<<'\n'<<std::flush;
        for(auto& p:policy.p) if(!(std::cin>>p.work>>p.value)) return 1;
        if(!(std::cin>>money)) return 1;
        for(auto& x:offer) if(!(std::cin>>x.type>>x.work>>x.price)) return 1;
        policy.observe(offer);
        int pick=0;
        best=-1e100;
        int queued=0;
        for(int i=0;i<n;++i) if(i!=c&&hand[i].type==4) ++queued;
        if(rem>1) for(int i=0;i<k;++i) {
            const auto& x=offer[i];
            if(x.price>money) continue;
            if(x.type==4&&(rem<140||policy.level+queued>=20)) continue;
#ifdef AHC029_GROWTH_RESERVE
            // Reserve 5 units at the new level, not 5 at the current level.
            if(x.type==4 && money-x.price<10*policy.unit()) continue;
#endif
            auto [value,where]=policy.target_for(x,rem-1,money);
            (void)where;
            double net=value-x.price;
            if(x.type==4) {
                // Compare prices if there is more than one growth offer.
                net=1e25-.001*double(x.price);
                if(pick&&offer[pick].type==4 && x.price>=offer[pick].price) continue;
                if(pick&&offer[pick].type==4) best=-1e100;
            }
            if(net>best) pick=i,best=net;
        }
        std::cout<<pick<<'\n'<<std::flush;
        hand[c]=offer[pick];money-=offer[pick].price;
    }
    return 0;
}
