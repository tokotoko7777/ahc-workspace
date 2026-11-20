#include <bits/stdc++.h>
#include "../../library/overlapping-interval-max-transform.hpp"
#include "../../library/manhattan-max-transform.hpp"
using namespace std;
#ifndef AHC039_OVERLAP
#define AHC039_OVERLAP 1
#endif
#ifndef AHC039_STRIPS
#define AHC039_STRIPS 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int G=40,S=100000,MAX_PERIM=400000;
constexpr long long SCALE=4096;
using Point=pair<int,int>;
struct Answer{
  int gain=-1,perimeter=0;
  vector<Point> polygon;
};
struct Grid{
  vector<int> xs,ys;
  array<array<int,G>,G> weight{};
  static vector<int> cuts(const vector<Point>& fish,bool row){
    vector<char> used(S+1);
    for(auto [x,y]:fish)used[row?y:x]=true;
    vector<int> a{0};
    for(int k=1;k<G;++k){
      int v=max(a.back()+1,k*S/G);
      while(v<S&&used[v])++v; // No fish lies on an interior cut.
      if(v>=S){
        vector<int> pool;
        for(int x=1;x<S;++x)if(!used[x])pool.push_back(x);
        a.resize(1);
        for(int j=1;j<G;++j)a.push_back(pool[j*pool.size()/G]);
        break;
      }
      a.push_back(v);
    }
    a.push_back(S);
    return a;
  }
  explicit Grid(const vector<Point>& fish){
    xs=cuts(fish,false);ys=cuts(fish,true);
    for(int i=0;i<int(fish.size());++i){
      auto [x,y]=fish[i];
      int a=min(G-1,int(upper_bound(xs.begin(),xs.end(),x)-xs.begin())-1);
      int b=min(G-1,int(upper_bound(ys.begin(),ys.end(),y)-ys.begin())-1);
      weight[a][b]+=i<int(fish.size())/2?1:-1;
    }
  }
  Grid transpose()const{
    Grid r=*this;
    swap(r.xs,r.ys);
    for(int i=0;i<G;++i)for(int j=0;j<G;++j)r.weight[i][j]=weight[j][i];
    return r;
  }
  Answer rectangle()const{
    Answer ans;
    for(int l=0;l<G;++l){
      array<int,G> line{};
      for(int r=l;r<G;++r){
        for(int j=0;j<G;++j)line[j]+=weight[r][j];
        int run=0,low=0;
        for(int u=0;u<G;++u){
          if(run<0){run=0;low=u;}
          run+=line[u];
          if(run<=ans.gain)continue;
          ans.gain=run;
          ans.perimeter=2*(xs[r+1]-xs[l]+ys[u+1]-ys[low]);
          ans.polygon={{xs[l],ys[low]},{xs[r+1],ys[low]},
                       {xs[r+1],ys[u+1]},{xs[l],ys[u+1]}};
        }
      }
    }
    return ans;
  }
  vector<Point> trace_shape(int last_x,int state,int n,int above,const vector<int>& hi,
                            const vector<int>& predecessor)const{
    const int num=n*above;
    vector<tuple<int,int,int>> sequence;
    for(int x=last_x;state>=0;--x){
      int l=state/above,h=state%above;
      sequence.emplace_back(x,ys[l],hi[h]);
      state=predecessor[x*num+state];
    }
    reverse(sequence.begin(),sequence.end());
    vector<Point> ans;
    auto push=[&](Point p){
      if(!ans.empty()&&ans.back()==p)return;
      while(ans.size()>=2){
        auto a=ans[ans.size()-2],b=ans.back();
        if((a.first==b.first&&b.first==p.first)||(a.second==b.second&&b.second==p.second))
          ans.pop_back();
        else break;
      }
      ans.push_back(p);
    };
    for(auto [x,l,h]:sequence){push({xs[x],l});push({xs[x+1],l});}
    for(auto i=sequence.rbegin();i!=sequence.rend();++i){
      auto [x,l,h]=*i;
      push({xs[x+1],h});push({xs[x],h});
    }
    return ans;
  }
  void strips(Answer& best,Clock::time_point deadline,bool extra=false)const{
    array<array<int,G+1>,G> prefix{};
    for(int x=0;x<G;++x)for(int y=0;y<G;++y)prefix[x][y+1]=prefix[x][y]+weight[x][y];
    for(int a=0;a<G;++a){
      vector<int> lo(ys.begin(),ys.begin()+a+1),hi(ys.begin()+a+1,ys.end());
      const int n=lo.size(),m=hi.size(),z=n*m;
      const vector<long long> penalties=extra?vector<long long>{3,5,9,14}:vector<long long>{0,1,2,4,7,11};
      for(long long penalty:penalties){
        if(Clock::now()>=deadline)return;
        vector<int> parent(z*G),old_raw(z),old_p(z),next_raw(z),next_p(z);
        vector<long long> value(z),d(z);
        vector<int> prev(z);
        for(int x=0;x<G;++x){
          d=value;
          for(int k=0;k<z;++k)prev[k]=x?k:-1;
          if(x)manhattan_max_transform(lo,hi,penalty,d,prev);
          const int width=xs[x+1]-xs[x];
          for(int l=0;l<n;++l)for(int h=0;h<m;++h){
            int k=l*m+h, length=hi[h]-lo[l],gain=prefix[x][a+1+h]-prefix[x][l];
            int q=prev[k];
            long long start=SCALE*gain-penalty*(2*width+length);
            long long carry=q<0?LLONG_MIN:d[k]+SCALE*gain-penalty*(2*width);
            if(carry>start){
              value[k]=carry;
              next_raw[k]=old_raw[q]+gain;
              next_p[k]=old_p[q]+2*width+abs(lo[l]-lo[q/m])+abs(hi[h]-hi[q%m]);
              parent[x*z+k]=q;
            }else{
              value[k]=start;next_raw[k]=gain;next_p[k]=2*width+length;parent[x*z+k]=-1;
            }
            const int p=next_p[k]+length;
            if(p<=MAX_PERIM&&(next_raw[k]>best.gain||
                (next_raw[k]==best.gain&&p<best.perimeter))){
              best.gain=next_raw[k];best.perimeter=p;best.polygon=trace_shape(x,k,n,m,hi,parent);
            }
          }
          old_raw.swap(next_raw);old_p.swap(next_p);
        }
      }
    }
  }
  void overlap(Answer& best,Clock::time_point deadline)const{
    constexpr int M=G+1,Z=M*M;
    array<array<int,M>,G> prefix{};
    for(int x=0;x<G;++x)for(int y=0;y<G;++y)prefix[x][y+1]=prefix[x][y]+weight[x][y];
    for(long long penalty:{0LL,1LL,2LL,4LL,7LL,11LL}){
      if(Clock::now()>=deadline)return;
      vector<int> parent(Z*G),old_raw(Z),old_p(Z),next_raw(Z),next_p(Z),prv(Z);
      vector<long long> cost(Z),trans(Z);
      for(int x=0;x<G;++x){
        trans=cost;
        fill(prv.begin(),prv.end(),-1);
        if(x){
          for(int l=0;l<M;++l)for(int h=l+1;h<M;++h)prv[l*M+h]=l*M+h;
          overlapping_interval_max_transform(ys,penalty,trans,prv);
        }
        const int width=xs[x+1]-xs[x];
        for(int l=0;l<M;++l)for(int h=l+1;h<M;++h){
          int k=l*M+h, delta=prefix[x][h]-prefix[x][l],length=ys[h]-ys[l], q=prv[k];
          long long start=SCALE*delta-penalty*(2*width+length);
          long long carry=q<0?LLONG_MIN:trans[k]+SCALE*delta-penalty*2*width;
          if(carry>start){
            assert(max(l,q/M)<min(h,q%M));
            cost[k]=carry;
            next_raw[k]=old_raw[q]+delta;
            next_p[k]=old_p[q]+2*width+abs(ys[l]-ys[q/M])+abs(ys[h]-ys[q%M]);
            parent[x*Z+k]=q;
          }else{
            cost[k]=start;next_raw[k]=delta;next_p[k]=2*width+length;parent[x*Z+k]=-1;
          }
          const int p=next_p[k]+length;
          if(p<=MAX_PERIM&&(next_raw[k]>best.gain||
              (next_raw[k]==best.gain&&p<best.perimeter))){
            best.gain=next_raw[k];best.perimeter=p;
            best.polygon=trace_shape(x,k,M,M,ys,parent);
          }
        }
        old_raw.swap(next_raw);old_p.swap(next_p);
      }
    }
  }

};
}
int main(){
  const auto deadline=Clock::now()+chrono::milliseconds(1450);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  int n;if(!(cin>>n))return 0;
  vector<Point> fish(2*n);
  for(auto& [x,y]:fish)cin>>x>>y;
  Grid g(fish);auto result=g.rectangle();const int base=result.gain;
  if(AHC039_STRIPS){
    g.strips(result,deadline);
    Answer rotated=result;
    g.transpose().strips(rotated,deadline);
    if(AHC039_OVERLAP==1){
      g.overlap(result,deadline);
      g.transpose().overlap(rotated,deadline);
    }else if(AHC039_OVERLAP==2){
      g.strips(result,deadline,true);
      g.transpose().strips(rotated,deadline,true);
    }
    if(pair{rotated.gain,-rotated.perimeter}>pair{result.gain,-result.perimeter}){
      result=std::move(rotated);
      for(auto& [x,y]:result.polygon)swap(x,y);
    }
  }
  assert(4<=result.polygon.size()&&result.polygon.size()<=1000&&result.perimeter<=MAX_PERIM);
  cout<<result.polygon.size()<<'\n';
  for(auto [x,y]:result.polygon)cout<<x<<' '<<y<<'\n';
  cerr<<"base="<<base<<" gain="<<result.gain<<" perimeter="<<result.perimeter<<'\n';
}
