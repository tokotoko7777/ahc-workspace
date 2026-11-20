#include <bits/stdc++.h>
#include "../../library/ragged-layer-shortest-path.hpp"
using namespace std;
#ifndef AHC028_EXACT
#define AHC028_EXACT 1
#endif
#ifndef AHC028_TOUR
#define AHC028_TOUR 0
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int C=225,M=200;
struct Problem {
  int n,m,start;
  array<vector<int>,26> cells;
  vector<string> word;
  array<array<uint8_t,C>,C> d{};
  vector<array<array<int16_t,C>,5>> suffix;
  int overlap[M][M]{};
  int proxy[M+1][M+1]{};
  void read(){
    int r,c;cin>>n>>m>>r>>c;start=r*n+c;
    for(int i=0;i<n;++i){
      string row;cin>>row;
      for(int j=0;j<n;++j)cells[row[j]-'A'].push_back(i*n+j);
    }
    word.resize(m);
    for(auto& w:word)cin>>w;
    for(int a=0;a<C;++a)for(int b=0;b<C;++b)d[a][b]=abs(a/n-b/n)+abs(a%n-b%n)+1;
    suffix.resize(m);
    for(int w=0;w<m;++w)for(int k=4;k>=0;--k)for(int a=0;a<C;++a){
      int best=INT_MAX;
      for(int b:cells[word[w][k]-'A'])
        best=min(best,int(d[a][b])+(k==4?0:suffix[w][k+1][b]));
      suffix[w][k][a]=best;
    }
    for(int a=0;a<=m;++a)for(int b=0;b<m;++b)if(a!=b){
      int k=0;
      if(a<m)for(int l=1;l<=4;++l)if(word[a].compare(5-l,l,word[b],0,l)==0)k=l;
      if(a==m){proxy[a][b]=16*suffix[b][k][start];continue;}
      overlap[a][b]=k;
      auto& points=cells[word[a].back()-'A'];
      int total=0;
      for(int s:points)total+=suffix[b][k][s];
      proxy[a][b]=(16*total+int(points.size())/2)/int(points.size());
    }
  }
  vector<int> greedy()const{
    vector<int> order; vector<bool> used(m);
    int at=start,last=m;
    for(int i=0;i<m;++i){
      int best=INT_MAX,chosen=-1,skip=0;
      for(int w=0;w<m;++w)if(!used[w]){
        int k=last==m?0:overlap[last][w];
        if(suffix[w][k][at]<best){best=suffix[w][k][at];chosen=w;skip=k;}
      }
      used[chosen]=true;order.push_back(chosen);last=chosen;
      for(int k=skip;k<5;++k){
        int bestCost=INT_MAX,newAt=-1;
        for(int id:cells[word[last][k]-'A']){
          int cost=int(d[at][id])+(k==4?0:suffix[last][k+1][id]);
          if(cost<bestCost){bestCost=cost;newAt=id;}
        }
        at=newAt;
      }
    }
    return order;
  }
  pair<int,vector<int>> type(const vector<int>& order)const{
    vector<const vector<int>*> layers;
    vector<int> widths;
    int prev=-1;
    for(int w:order){
      for(int k=prev<0?0:overlap[prev][w];k<5;++k){
        auto& point=cells[word[w][k]-'A'];
        layers.push_back(&point);widths.push_back(point.size());
      }
      prev=w;
    }
    auto [cost,path]=ragged_layer_shortest_path<int>(widths,
      [&](int q){return int(d[start][(*layers[0])[q]]);},
      [&](int l,int p,int q){return int(d[(*layers[l-1])[p]][(*layers[l])[q]]);});
    for(int i=0;i<int(path.size());++i)path[i]=(*layers[i])[path[i]];
    return {cost,std::move(path)};
  }
  using Frontier=vector<int>;
  const vector<int>& keys(int w,int k)const{return cells[word[w][k]-'A'];}
  Frontier advance(const Frontier& prior,const vector<int>& from,const vector<int>& to)const{
    Frontier f(to.size(),INT_MAX);
    for(size_t i=0;i<from.size();++i)for(size_t j=0;j<to.size();++j)
      f[j]=min(f[j],prior[i]+d[from[i]][to[j]]);
    return f;
  }
  Frontier backward(const Frontier& after,const vector<int>& from,const vector<int>& to)const{
    Frontier f(from.size(),INT_MAX);
    for(size_t i=0;i<from.size();++i)for(size_t j=0;j<to.size();++j)
      f[i]=min(f[i],after[j]+d[from[i]][to[j]]);
    return f;
  }
  struct Frontiers{vector<Frontier> prefix,suffix;};
  Frontiers make_frontiers(const vector<int>& order)const{
    Frontiers f{vector<Frontier>(m+1),vector<Frontier>(m)};
    f.prefix[0]={0};
    const vector<int> start_key{start};
    for(int i=0;i<m;++i){
      auto* from=i?&keys(order[i-1],4):&start_key;
      auto val=f.prefix[i];
      for(int k=i?overlap[order[i-1]][order[i]]:0;k<5;++k){
        auto& to=keys(order[i],k);
        val=advance(val,*from,to);from=&to;
      }
      f.prefix[i+1]=std::move(val);
    }
    f.suffix[m-1].resize(keys(order.back(),4).size());
    for(int i=m-2;i>=0;--i){
      auto val=f.suffix[i+1];
      int skip=overlap[order[i]][order[i+1]];
      for(int k=3;k>=skip;--k)
        val=backward(val,keys(order[i+1],k),keys(order[i+1],k+1));
      f.suffix[i]=backward(val,keys(order[i],4),keys(order[i+1],skip));
    }
    return f;
  }
  int edit_score(const vector<int>& edited,const Frontiers& f,int left,int right)const{
    // Include the first unchanged word on the right, whose incoming overlap
    // may have changed. Once its last letter is reached, reuse its tail.
    right=min(m-1,right+1);
    const vector<int> start_key{start};
    const vector<int>* from=left?&keys(edited[left-1],4):&start_key;
    auto val=f.prefix[left];
    for(int i=left;i<=right;++i){
      for(int k=i?overlap[edited[i-1]][edited[i]]:0;k<5;++k){
        auto& to=keys(edited[i],k);
        val=advance(val,*from,to);from=&to;
      }
    }
    if(right==m-1)return *min_element(val.begin(),val.end());
    int best=INT_MAX;
    for(size_t j=0;j<val.size();++j)best=min(best,val[j]+f.suffix[right][j]);
    return best;
  }
  pair<int,vector<int>> exact_search(vector<int> order,Clock::time_point deadline)const{
    auto best=type(order);
    auto f=make_frontiers(order);
    mt19937 rng(42028);
    for(int iteration=0;iteration<18000;++iteration){
      if((iteration&63)==0&&Clock::now()>=deadline)break;
      int left=rng()%(m-1);
      int right=min(m-1,left+1+int(rng()%8));
      int mode=rng()%3;
      auto trial=order;
      if(mode==0)swap(trial[left],trial[right]);
      else if(mode==1)rotate(trial.begin()+left,trial.begin()+left+1,trial.begin()+right+1);
      else rotate(trial.begin()+left,trial.begin()+right,trial.begin()+right+1);
      int score=edit_score(trial,f,left,right);
#ifdef AHC028_VERIFY_FRONTIERS
      if(iteration%11==0)assert(score==type(trial).first);
#endif
      if(score>=best.first)continue;
      best=type(trial);
      assert(best.first==score);
      order.swap(trial);
      f=make_frontiers(order);
    }
    return best;
  }
  pair<int,vector<int>> search(vector<int> base,bool improve,Clock::time_point deadline)const{
    auto best=type(base);
    if(!improve)return best;
    mt19937 rng(289123);
    for(int epoch=0;epoch<4;++epoch){
      vector<int> ord=base;ord.insert(ord.begin(),m);ord.push_back(m);
      if(epoch%2)shuffle(ord.begin()+1,ord.end()-1,rng);
      int val=0;
      for(int k=1;k<=m+1;++k)val+=proxy[ord[k-1]][ord[k]];
      int record=val;
      vector<int> record_order=ord;
      for(int step=0;step<52000;++step){
        if((step&511)==0&&Clock::now()>=deadline)break;
        const int i=1+rng()%m,j=1+rng()%m;
        if(i==j)continue;
        int delta=0;
        bool sw=(rng()&3)==0;
        if(sw){
          array<int,4> e{i-1,i,j-1,j};sort(e.begin(),e.end());
          for(int k=0;k<4;++k)if(k==0||e[k]!=e[k-1]){
            int x=e[k],a=ord[x],b=ord[x+1];
            int A=x==i?ord[j]:x==j?ord[i]:a;
            int B=x+1==i?ord[j]:x+1==j?ord[i]:b;
            delta+=proxy[A][B]-proxy[a][b];
          }
        }else{
          // Remove i, insert it just after j in the original indexing.
          if(i==j+1)continue;
          int a=ord[i-1],x=ord[i],b=ord[i+1],c=ord[j],d=ord[j+1];
          delta=proxy[a][b]+proxy[c][x]+proxy[x][d]-proxy[a][x]-proxy[x][b]-proxy[c][d];
        }
        const double fraction=step/52000.0;
        const double temp=(epoch==0?26.:46.)*(1-fraction)*(1-fraction)+1.0;
        if(delta>0&&double(rng())/4294967296.0>=exp(-delta/temp))continue;
        if(sw)swap(ord[i],ord[j]);
        else if(i<j)rotate(ord.begin()+i,ord.begin()+i+1,ord.begin()+j+1);
        else rotate(ord.begin()+j+1,ord.begin()+i,ord.begin()+i+1);
        val+=delta;
#ifdef AHC028_VERIFY_DELTAS
        int check=0;
        for(int k=1;k<=m+1;++k)check+=proxy[ord[k-1]][ord[k]];
        assert(val==check);
#endif
        if(val<record){record=val;record_order=ord;}
        if((step%251)==0&&step>20000){
          auto p=type(vector<int>(ord.begin()+1,ord.end()-1));
          if(p.first<best.first)best=std::move(p);
        }
      }
      vector<int> picked(record_order.begin()+1,record_order.end()-1);
      auto p=type(picked);
      if(p.first<best.first){best=std::move(p);base=picked;}
      if(Clock::now()>=deadline)break;
    }
    return best;
  }
};
}
#ifndef AHC028_TEST
int main(){
  const auto deadline=Clock::now()+chrono::milliseconds(1550);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();
  const auto [cost,path]=AHC028_EXACT?p.exact_search(p.greedy(),deadline):p.search(p.greedy(),AHC028_TOUR,deadline);
  for(int id:path)cout<<id/p.n<<' '<<id%p.n<<'\n';
  cerr<<"cost="<<cost<<'\n';
}
#endif
