#include "../library/linear-gaussian-updater.hpp"
#include <cassert>
#include <cmath>
#include <random>
#include <utility>
#include <vector>
using namespace std;
int main(){
  mt19937 rng(40401);
  for(int t=0;t<700;++t){
    const int n=1+rng()%7, m=1+rng()%15;
    vector<double> prior(n);
    for(double& x:prior)x=int(rng()%21)-10;
    const double var=1+rng()%10;
    LinearGaussianUpdater p(prior,var);
    vector<vector<double>> system(n,vector<double>(n+1));
    for(int i=0;i<n;++i){system[i][i]=1./var;system[i][n]=prior[i]/var;}
    for(int k=0;k<m;++k){
      vector<double> a(n);
      vector<pair<int,double>> terms;
      for(int i=0;i<n;++i)if(rng()%3){a[i]=int(rng()%11)-5;terms.emplace_back(i,a[i]);}
      double obs=int(rng()%61)-30,no=1+rng()%9;
      p.observe(terms,obs,no);
      for(int i=0;i<n;++i){
        for(int j=0;j<n;++j)system[i][j]+=a[i]*a[j]/no;
        system[i][n]+=a[i]*obs/no;
      }
    }
    auto row=system;
    for(int i=0;i<n;++i){
      const double z=row[i][i];assert(z>0);
      for(int j=0;j<=n;++j)row[i][j]/=z;
      for(int k=0;k<n;++k)if(k!=i){
        const double f=row[k][i];
        for(int j=0;j<=n;++j)row[k][j]-=row[i][j]*f;
      }
    }
    for(int i=0;i<n;++i){
      assert(abs(p.mean()[i]-row[i][n])<1e-8);
      for(int j=0;j<n;++j){
        double v=0;
        for(int k=0;k<n;++k)v+=system[i][k]*p.covariance()[k*n+j];
        assert(abs(v-double(i==j))<1e-8);
      }
    }
  }
}
