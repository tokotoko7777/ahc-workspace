#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <span>
#include <vector>
#include "../../library/budgeted-order-buckets.hpp"

struct Scale {
    int queries;
    int cmp(std::span<const int> a,std::span<const int> b) {
        if(a.empty()||b.empty()) return int(!a.empty())-int(!b.empty());
        if(queries<=0) std::exit(1);
        --queries;
        std::cout<<a.size()<<' '<<b.size();
        for(int x:a) std::cout<<' '<<x;
        for(int x:b) std::cout<<' '<<x;
        std::cout<<'\n'<<std::flush;
        char sign;
        if(!(std::cin>>sign) || (sign!='<'&&sign!='>'&&sign!='=')) std::exit(1);
        return (sign=='>')-(sign=='<');
    }
    int single(int a,int b) {
        return cmp(std::span(&a,1),std::span(&b,1));
    }
};
#ifdef AHC025_GROUP_INFORMATION
struct SignScheduler {
    std::vector<double> mean,variance;
    static double cdf(double z) {return .5*std::erfc(-.7071067811865475*z);}
    static double pdf(double z) {return .3989422804014327*std::exp(-.5*z*z);}
    std::pair<double,double> gap(const std::vector<int>& a,const std::vector<int>& b) const {
        double m=0.,v=0.;
        for(int i:a) m+=mean[i],v+=variance[i];
        for(int i:b) m-=mean[i],v+=variance[i];
        return {m,std::max(v,1e-12)};
    }
    double information(const std::vector<int>& a,const std::vector<int>& b) const {
        const auto [m,v]=gap(a,b);
        const double z=m/std::sqrt(v),p=cdf(z),r=cdf(-z),f=pdf(z);
        return std::min(p,r)<1e-14 ? 0. : v*f*f/(p*r);
    }
    void observe(const std::vector<int>& a,const std::vector<int>& b,int sign) {
        const auto [m,v]=gap(a,b);
        const double sd=std::sqrt(v);
        double shift=-m,shrink=1.;
        if(sign) {
            const double z=sign*m/sd,p=cdf(z);
            const double f=p>1e-14 ? pdf(z)/p : -z+1./std::max(1.,-z);
            shift=sign*sd*f;
            shrink=std::clamp(f*(f+z),0.,1.);
        }
        auto change=[&](int i,int side) {
            const double fraction=variance[i]/v;
            mean[i]+=side*fraction*shift;
            variance[i]*=1.-fraction*shrink;
        };
        for(int i:a) change(i,1);
        for(int i:b) change(i,-1);
    }
};
#endif
struct Balancer {
    int n,d;
    std::vector<double> w,load;
    std::vector<int> rank,where,version;
    std::vector<std::vector<int>> group;
#ifdef AHC025_GROUP_INFORMATION
    SignScheduler scheduler;
#endif
    Balancer(int n,int d):n(n),d(d),w(n),load(d),rank(n),where(n),version(d),group(d){}
    void initialize(const std::vector<std::vector<int>>& buckets,std::mt19937& rng) {
        const double truncate=-std::expm1(-double(n)/d);
        int rank_index=0,offset=0;
#ifdef AHC025_GROUP_INFORMATION
        scheduler.mean.resize(n);
        scheduler.variance.resize(n);
#endif
        for(const auto& bucket:buckets) {
            double mean=0;
            for(int i=0;i<int(bucket.size());++i)
                mean-=std::log1p(-truncate*(offset+i+.5)/n);
            mean/=bucket.size();
            for(int id:bucket) {w[id]=mean;rank[id]=rank_index;}
#ifdef AHC025_GROUP_INFORMATION
            double v=0.;
            for(int i=0;i<int(bucket.size());++i) {
                const double p=(offset+i+.5)/n,deriv=truncate/(1.-truncate*p);
                const double diff=-std::log1p(-truncate*p)-mean;
                v+=p*(1.-p)/(n+2.)*deriv*deriv+diff*diff;
            }
            for(int id:bucket) scheduler.mean[id]=mean,scheduler.variance[id]=v/bucket.size();
#endif
            offset+=bucket.size();++rank_index;
        }
        for(auto it=buckets.rbegin();it!=buckets.rend();++it) for(int id:*it) {
            int g=std::min_element(load.begin(),load.end())-load.begin();
            where[id]=g;load[g]+=w[id];
        }
        for(int iter=0;iter<20000;++iter) {
            int a=rng()%n,b=rng()%n,ga=where[a],gb=where[b];
            if(ga==gb) continue;
            double delta=w[a]-w[b];
            if(delta*(load[ga]-load[gb]-delta)<=1e-13) continue;
            std::swap(where[a],where[b]);load[ga]-=delta;load[gb]+=delta;
        }
        for(int i=0;i<n;++i) group[where[i]].push_back(i);
    }
    void balance(Scale& scale,std::mt19937& rng) {
        struct Cache {
            int va=-1,vb=-1,sign=0;
            std::vector<std::pair<int,int>> tried;
        };
        std::vector<Cache> history(d*d);
        const int max_attempts=5*scale.queries;
#ifdef AHC025_GROUP_INFORMATION
        const int early_end=scale.queries/2;
        auto priority=[&](int a,int b,bool early) {
            if(!early) return std::abs(load[a]-load[b]);
            if(a==b) return -1.;
            if(a>b) std::swap(a,b);
            auto& saved=history[a*d+b];
            if(saved.va==version[a] && saved.vb==version[b]) return 0.;
            return scheduler.information(group[a],group[b]);
        };
#endif
        for(int attempt=0;attempt<max_attempts&&scale.queries>=2;++attempt) {
            int a=rng()%d,b=rng()%d;
            if(a==b) continue;
#ifdef AHC025_GROUP_INFORMATION
            const bool early=scale.queries>early_end;
#endif
            if(attempt%3==0) for(int tries=0;tries<2;++tries) {
                int x=rng()%d,y=rng()%d;
#ifdef AHC025_GROUP_INFORMATION
                if(priority(x,y,early)>priority(a,b,early)) a=x,b=y;
#else
                if(std::abs(load[x]-load[y])>std::abs(load[a]-load[b])) a=x,b=y;
#endif
            }
            if(a>b) std::swap(a,b);
            auto& cache=history[a*d+b];
            if(cache.va!=version[a]||cache.vb!=version[b]) {
                cache={version[a],version[b],scale.cmp(group[a],group[b]),{}};
#ifdef AHC025_GROUP_INFORMATION
                if(early) scheduler.observe(group[a],group[b],cache.sign);
#endif
            }
            if(!cache.sign) continue;
            if(cache.sign<0) std::swap(a,b);
            int best_a=-1,best_b=-1;
            double best=1e100;
            const double target=std::abs(load[a]-load[b])*.5+
                                (0.035+0.16*std::generate_canonical<double,32>(rng))*std::sqrt(double(n)/d);
            auto consider=[&](int x,int y,double delta) {
                if(std::find(cache.tried.begin(),cache.tried.end(),std::pair{x,y})!=cache.tried.end()) return;
                const auto loss=std::abs(delta-target)+(delta>target?.16:0.)*delta+
                                 .018*std::generate_canonical<double,32>(rng);
                if(loss<best) best=loss,best_a=x,best_b=y;
            };
            for(int x:group[a]) {
                if(group[a].size()>1) consider(x,-1,w[x]);
                for(int y:group[b]) if(rank[x]>rank[y])
                    consider(x,y,w[x]-w[y]);
            }
            if(best_a<0) continue;
            const int x=best_a,y=best_b;
            cache.tried.emplace_back(x,y);
            std::vector<int> lhs,rhs;
            for(int i:group[a]) if(i!=x) lhs.push_back(i);
            for(int i:group[b]) if(i!=y) rhs.push_back(i);
            // The rank bucket order proves that the selected net transfer is
            // positive. A−x >= B−y then bounds it by the old group gap.
            if(scale.cmp(lhs,rhs)<0) continue;
            lhs.push_back(y); rhs.push_back(x);
            if(y<0) lhs.pop_back(); else where[y]=a;
            where[x]=b;
            group[a]=std::move(lhs);group[b]=std::move(rhs);
            const double transfer=w[x]-(y<0?0.:w[y]);
            load[a]-=transfer; load[b]+=transfer;
            ++version[a];++version[b];
        }
    }
};

int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    int n,d,q;
    if(!(std::cin>>n>>d>>q)) return 1;
    Scale scale{q};
    std::mt19937 rng(5911291);
    std::vector<int> items(n);std::iota(items.begin(),items.end(),0);
#ifndef AHC025_RANK_ONLY
    int reserve=q<3*n?q/7:q<5*n?q/5:q/3;
#else
    int reserve=0;
#endif
    auto ordered=budgeted_order_buckets(std::move(items),q-reserve,
        [&](int a,int b){return scale.single(a,b);},rng);
    Balancer b(n,d);
    b.initialize(ordered,rng);
#ifndef AHC025_RANK_ONLY
    b.balance(scale,rng);
#endif
    while(scale.queries) scale.single(0,1);
    for(int i=0;i<n;++i) std::cout<<b.where[i]<<(i+1==n?'\n':' ');
    std::cout<<std::flush;
}
