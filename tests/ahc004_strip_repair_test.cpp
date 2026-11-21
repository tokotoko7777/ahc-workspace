#pragma GCC optimize("O3")
#include "../problems/ahc004/strip-repair.hpp"
#include <cassert>
#include <iostream>
#include <random>
using namespace std;
using namespace ahc004::strip;

int main(){
  mt19937 rng(43738675);
  auto stop=[](){return false;};
  int pair_kept=0,single_kept=0,right=0,wrong=0,same_as_wounded=0;
  for(int t=0;t<48;++t){
    Board truth(20,string(20,' '));
    for(auto& row:truth)for(char& c:row)c='A'+rng()%8;
    vector<string> reads;
    for(int i=0;i<640;++i){
      int r=rng()%20,c=rng()%20,d=rng()%2,len=5+rng()%5;
      string w;
      for(int j=0;j<len;++j)w+=truth[(r+d*j)%20][(c+(1-d)*j)%20];
      reads.push_back(w);
    }
    vector<int> weights(reads.size(),1);
    Board wounded=truth;
    for(auto& row:wounded)for(char& c:row)if(rng()%100>=81)c='.';
    int a=rng()%40,b=-1;
    if(t>=24){b=rng()%40;if(b==a)b=(a+1)%40;}
    for(int l:{a,b})if(l>=0)for(int p=0;p<20;++p){
      auto& c=l<20?wounded[l][p]:wounded[p][l-20];
      if(c!='.')c='A'+(c-'A'+1+rng()%7)%8;
    }
    Evidence x(wounded,reads,weights,stop);
    assert(x.complete);
    auto choices=x.rank(wounded,stop);
    for(auto [i,j]:vector<pair<int,int>>{{-1,-1},{a,-1},{a,b},{int(rng()%40),int(rng()%40)}}){
      auto cand=x.blank(wounded,Choice{i,j});
      int bins[3]{};
      for(const auto& word:reads){
        int best=3;
        for(int r=0;r<20;++r)for(int c=0;c<20;++c)for(int d=0;d<2;++d){
          int holes=0;
          for(int k=0;k<int(word.size());++k){
            char x=cand[(r+k*d)%20][(c+k*(1-d))%20];
            if(x=='.')++holes;
            else if(x!=word[k]){holes=100;break;}
          }
          best=min(best,holes);
        }
        if(best<3)++bins[best];
      }
      const auto val=x.assess(i,j);
      assert(val.exact==bins[0] && val.one==bins[1] && val.two==bins[2]);
    }
    for(const auto& c:choices){
      if(t<24 ? (c.a==a && c.b<0) : ((c.a==a && c.b==b)||(c.a==b&&c.b==a))){
        (t<24?single_kept:pair_kept)++;
        auto blank=x.blank(wounded,c);
        auto [repaired,filled]=x.seed(wounded,c);
        int counted=0;
        for(int r=0;r<20;++r)for(int col=0;col<20;++col){
          if(blank[r][col]!='.'){assert(blank[r][col]==repaired[r][col]);continue;}
          if(repaired[r][col]=='.')continue;
          if(repaired[r][col]==truth[r][col])++right;else ++wrong;
          same_as_wounded+=repaired[r][col]==wounded[r][col];++counted;
        }
        assert(counted==filled);
        break;
      }
    }
  }
  cout<<"oracle=192 single_kept="<<single_kept<<"/24 pair_kept="<<pair_kept
        <<"/24 correct_guesses="<<right<<" wrong_guesses="<<wrong
        <<" repeated_corrupt="<<same_as_wounded<<"\n";
}
