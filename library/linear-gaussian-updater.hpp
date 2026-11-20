#pragma once
#include <cassert>
#include <utility>
#include <vector>

// Gaussian mean/covariance with independent measurement errors and scalar
// linear observations. Coefficients are sparse (index, weight) pairs.
class LinearGaussianUpdater {
  std::vector<double> mean_,cov_;
public:
  LinearGaussianUpdater(std::vector<double> mean,double initial_variance)
    :mean_(std::move(mean)),cov_(mean_.size()*mean_.size()){
    assert(initial_variance>0);
    for(std::size_t i=0;i<mean_.size();++i)cov_[i*mean_.size()+i]=initial_variance;
  }
  const std::vector<double>& mean()const{return mean_;}
  const std::vector<double>& covariance()const{return cov_;}
  void observe(const std::vector<std::pair<int,double>>& coefficients,
               double actual,double noise_variance){
    assert(noise_variance>0);
    const int n=int(mean_.size());
    std::vector<double> v(n);
    double estimate=0;
    for(auto [i,w]:coefficients){
      assert(0<=i&&i<n);
      estimate+=w*mean_[i];
      for(int j=0;j<n;++j)v[j]+=cov_[j*n+i]*w;
    }
    double den=noise_variance;
    for(auto [i,w]:coefficients)den+=w*v[i];
    assert(den>0);
    for(int i=0;i<n;++i){
      double scaled=v[i]/den;
      mean_[i]+=scaled*(actual-estimate);
      for(int j=0;j<n;++j)cov_[i*n+j]-=scaled*v[j];
    }
  }
};
