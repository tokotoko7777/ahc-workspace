#include "../library/markov-pattern-probability.hpp"
#include <cassert>
#include <cmath>
#include <functional>
#include <random>
#include <string>
#include <utility>
#include <vector>

int main(){
  std::mt19937 rng(470947);
  for(int t=0;t<1000;++t){
    int n=1+rng()%4,L=1+rng()%8,len=1+rng()%5,start=rng()%n;
    std::string labels(n,'a'),word(len,'a');
    for(char& c:labels)c+=rng()%3;
    for(char& c:word)c+=rng()%3;
    std::vector<double> a(n*n);
    for(int i=0;i<n;++i){
      int sum=0;
      for(int j=0;j<n;++j){a[i*n+j]=1+rng()%6;sum+=a[i*n+j];}
      for(int j=0;j<n;++j)a[i*n+j]/=sum;
    }
    double actual=0.;
    std::function<void(int,std::string,double)> brute=[&](int pos,std::string s,double p){
      if(s.find(word)!=std::string::npos){actual+=p;return;}
      if(int(s.size())==L)return;
      for(int q=0;q<n;++q)brute(q,s+labels[q],p*a[pos*n+q]);
    };
    brute(start,std::string(1,labels[start]),1.);
    assert(std::abs(markov_pattern_probability(labels,a,start,L,word)-actual)<1e-11);
  }
}
