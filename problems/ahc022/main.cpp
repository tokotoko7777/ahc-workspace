#include <bits/stdc++.h>
#include "../../library/hungarian.hpp"
#include "../../library/rounded-clipped-normal.hpp"
using namespace std;

#ifndef AHC022_STENCIL
#define AHC022_STENCIL 1
#endif
#ifndef AHC022_LOW_ENERGY
#define AHC022_LOW_ENERGY 1
#endif

namespace {
using Clock=chrono::steady_clock;
constexpr int LIMIT=10000,MAXTEMP=1000;
struct Rng {
  uint64_t x=0x22a8ee4b521;
  unsigned next(){x^=x<<13;x^=x>>7;x^=x<<17;return x>>19;}
};

#if AHC022_LOW_ENERGY || defined(AHC022_ENERGY_TEST)
#include "low-energy.hpp"
#endif

struct Puzzle {
  int l,n,s,used=0;
  vector<pair<int,int>> exit;
  vector<int> temp;
  Clock::time_point deadline=Clock::now()+chrono::milliseconds(3620);

  int cell(int y,int x)const{return ((y+l)%l)*l+(x+l)%l;}
  void read(){
    cin>>l>>n>>s;exit.resize(n);temp.assign(l*l,500);
    for(auto& [y,x]:exit)cin>>y>>x;
  }
  void smooth(const vector<char>& fixed) {
    for(int sweep=0;sweep<100;++sweep)
      for(int y=0;y<l;++y)for(int x=0;x<l;++x){
        int i=y*l+x;
        if(!fixed[i])temp[i]=(temp[cell(y-1,x)]+temp[cell(y+1,x)]
                              +temp[cell(y,x-1)]+temp[cell(y,x+1)]+2)/4;
      }
  }
  void placement()const {
    for(int y=0;y<l;++y){
      for(int x=0;x<l;++x)cout<<(x?" ":"")<<temp[y*l+x];
      cout<<'\n';
    }
    cout.flush();
  }
  int measure(int i,int y,int x) {
    assert(used<LIMIT && abs(y)<l && abs(x)<l);
    ++used;
    cout<<i<<' '<<y<<' '<<x<<endl;
    int value;
    if(!(cin>>value)||value<0)std::exit(0);
    return value;
  }

  vector<int> scalar() {
    vector<int> order(n),temperature(n);
    iota(order.begin(),order.end(),0);
    sort(order.begin(),order.end(),[&](int i,int j){
      auto metric=[&](int a){
        return sin(2*numbers::pi*exit[a].second/l)
               +0.37*cos(2*numbers::pi*exit[a].first/l);
      };
      return pair{metric(i),i}<pair{metric(j),j};
    });
    const int gap=clamp(int(2.25*s),1,MAXTEMP/(n-1));
    const int mean=(MAXTEMP-(n-1)*gap)/2;
    vector<char> fixed(l*l);
    for(int i=0;i<n;++i){
      auto [y,x]=exit[order[i]];
      int p=cell(y,x);
      fixed[p]=true;
      temp[p]=temperature[order[i]]=mean+i*gap;
    }
    smooth(fixed);
    placement();
    vector<vector<double>> table(n,vector<double>(MAXTEMP+1)),cost(n,vector<double>(n));
    for(int j=0;j<n;++j)for(int z=0;z<=MAXTEMP;++z)
      table[j][z]=-rounded_clipped_normal_logmass(z,temperature[j],s,0,MAXTEMP);
    const int reps=clamp(int(ceil(pow(1.1*s/gap,2))),2,LIMIT/n);
    for(int r=0;r<reps && Clock::now()<deadline;++r)for(int i=0;i<n;++i){
      int z=measure(i,0,0);
      for(int j=0;j<n;++j)cost[i][j]+=table[j][z];
    }
    return hungarian_minimum_assignment(cost);
  }

  vector<int> stencil() {
    vector<pair<int,int>> offsets;
    const int d=s<=45?11:s<=220?17:23;
    for(int dist=0;dist<=3 && int(offsets.size())<d;++dist){
      for(int y=-dist;y<=dist && int(offsets.size())<d;++y){
        int x=dist-abs(y);
        offsets.emplace_back(y,x);
        if(x && int(offsets.size())<d)offsets.emplace_back(y,-x);
      }
    }
    assert(int(offsets.size())==d);
    vector<vector<int>> location(n,vector<int>(d));
    vector<char> fixed(l*l);
    for(int i=0;i<n;++i)for(int b=0;b<d;++b){
      int at=cell(exit[i].first+offsets[b].first,exit[i].second+offsets[b].second);
      location[i][b]=at;
      fixed[at]=true;
    }
    vector<char> pattern,attempt(l*l);
    vector<unsigned> code(n),words(n);
    Rng rng;
    double best=-1e100;
    for(int trial=0;trial<80 && Clock::now()<deadline;++trial){
      for(char& z:attempt)z=rng.next()&1;
      for(int i=0;i<n;++i){
        words[i]=0;
        for(int b=0;b<d;++b)words[i]|=unsigned(attempt[location[i][b]])<<b;
      }
      double diversity=0;
      for(int i=0;i<n;++i)for(int j=i+1;j<n;++j){
        int dist=popcount(words[i]^words[j]);
        diversity-=(dist==0?30000.0:1.0/(dist*dist));
      }
      if(diversity>best)best=diversity,pattern=attempt,code=words;
    }
    assert(!pattern.empty());
    const int delta=clamp(int(1.85*s),6,500);
    const auto layout=[&](const vector<char>& bits){
      fill(temp.begin(),temp.end(),500);
      for(int i=0;i<l*l;++i)if(fixed[i])temp[i]=500+(bits[i]?delta:-delta);
      smooth(fixed);
      long long cost=0;
      for(int y=0;y<l;++y)for(int x=0;x<l;++x){
        long long a=temp[cell(y,x)]-temp[cell(y+1,x)];
        long long b=temp[cell(y,x)]-temp[cell(y,x+1)];
        cost+=a*a+b*b;
      }
      return cost;
    };
    const auto before=layout(pattern);
#if AHC022_LOW_ENERGY
    const auto previous_grid=temp;
    const auto previous_code=code;
    reduce_energy(l,fixed,location,pattern,code,min(deadline,Clock::now()+chrono::milliseconds(620)));
    if(layout(pattern)>before)temp=previous_grid,code=previous_code;
#endif
    placement();
    vector<double> odds(MAXTEMP+1);
    for(int z=0;z<=MAXTEMP;++z)
      odds[z]=rounded_clipped_normal_logmass(z,500-delta,s,0,MAXTEMP)
             -rounded_clipped_normal_logmass(z,500+delta,s,0,MAXTEMP);
    vector<vector<double>> cost(n,vector<double>(n));
    vector<vector<int>> seen(n,vector<int>(d));
    auto sample=[&](int i,int b){
      auto [y,x]=offsets[b];
      int z=measure(i,y,x);
      ++seen[i][b];
      for(int j=0;j<n;++j)
        if((code[j]>>b)&1)cost[i][j]+=odds[z];
    };
    int warm=clamp(int(ceil(0.55*pow(double(s)/delta,2))),1,3);
    for(int pass=0;pass<warm;++pass)for(int i=0;i<n;++i)for(int b=0;b<d;++b)
      if(used<LIMIT && Clock::now()<deadline)sample(i,b);

    while(used<LIMIT && Clock::now()<deadline){
      auto assign=hungarian_minimum_assignment(cost);
      int before=used;
      for(int i=0;i<n && used<LIMIT && Clock::now()<deadline;++i){
        int a=assign[i];
        const auto& row=cost[i];
        vector<int> alternatives;
        for(int j=0;j<n;++j)if(j!=a)alternatives.push_back(j);
        const int count=min(8,int(alternatives.size()));
        partial_sort(alternatives.begin(),alternatives.begin()+count,alternatives.end(),
                     [&](int j,int k){return row[j]<row[k];});
        if(row[alternatives[0]]-row[a]>=7.0)continue;
        double value=-1;
        int pick=-1;
        for(int b=0;b<d;++b){
          double impact=0;
          for(int t=0;t<count;++t){
            int j=alternatives[t];
            if(((code[a]^code[j])>>b)&1)
              impact+=exp(-min(24.0,max(0.0,row[j]-row[alternatives[0]]))*0.45);
          }
          const auto [y,x]=offsets[b];
          impact/=(10+abs(y)+abs(x))*(1+0.32*seen[i][b]);
          if(impact>value)value=impact,pick=b;
        }
        if(value>0)sample(i,pick);
      }
      if(before==used)break;
    }
    return hungarian_minimum_assignment(cost);
  }
};
} // namespace

#ifndef AHC022_NO_MAIN
int main(){
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  Puzzle p{};
  p.read();
  auto answer=AHC022_STENCIL?p.stencil():p.scalar();
  cout<<"-1 -1 -1\n";
  for(int x:answer)cout<<x<<'\n';
  cout.flush();
}
#endif
