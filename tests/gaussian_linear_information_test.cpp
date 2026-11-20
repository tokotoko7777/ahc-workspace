#include "../library/gaussian-linear-information.hpp"
#include <cassert>
#include <cmath>
#include <random>
#include <vector>

int main() {
    std::mt19937 engine(18001);
    std::uniform_real_distribution<double> num(-2., 2.);
    for(int trial=0;trial<1000;++trial){
        int n=1+engine()%8;
        std::vector<double> r(n*n), sigma(n*n), x(n), d(n);
        for(auto& v : r) v=num(engine);
        for(int i=0;i<n;++i) for(int j=0;j<n;++j) for(int k=0;k<n;++k)
            sigma[i*n+j]+=r[i*n+k]*r[j*n+k];
        std::vector<std::pair<int,double>> sparse, target;
        for(int i=0;i<n;++i) if(engine()%3){x[i]=num(engine); sparse.push_back({i,x[i]});}
        for(int i=0;i<n;++i) if(engine()%3){d[i]=num(engine); target.push_back({i,d[i]});}
        double noise=0.1+std::abs(num(engine));
        const auto res=gaussian_linear_information::expected_reduction(n,sigma,sparse,noise);
        // Independent reference: compute updated covariance via its explicit
        // dense rank-one Bayesian update and compare the difference in traces.
        std::vector<double> col(n);
        double signal=0., trace_old=0., trace_new=0., target_old=0., target_new=0.;
        for(int i=0;i<n;++i) for(int j=0;j<n;++j){
            signal+=x[i]*sigma[i*n+j]*x[j];
            col[i]+=sigma[i*n+j]*x[j];
        }
        for(int i=0;i<n;++i) for(int j=0;j<n;++j){
            const double updated=sigma[i*n+j]-col[i]*col[j]/(noise+signal);
            target_old += d[i]*sigma[i*n+j]*d[j];
            target_new += d[i]*updated*d[j];
            if(i==j){trace_old+=sigma[i*n+i];trace_new+=updated;}
        }
        assert(std::abs(res.signal_variance-signal)<1e-10);
        assert(std::abs(res.trace_drop-(trace_old-trace_new))<1e-10);
        assert(res.trace_drop>=0.0);
        const double targeted = gaussian_linear_information::target_variance_drop(n,sigma,sparse,target,noise);
        assert(std::abs(targeted-(target_old-target_new))<1e-10);
        assert(targeted>=0.0);
    }
}
