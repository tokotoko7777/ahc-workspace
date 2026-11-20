#include <bits/stdc++.h>
using namespace std;
#ifndef AHC072_CARAVAN
#define AHC072_CARAVAN 1
#endif
#ifndef AHC072_BATCH
#define AHC072_BATCH 0
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int V=400;
constexpr char directions[]="UDLR";
using Ray=array<array<int,2>,4>;
using Sheet=array<int,V>;
struct Move{int p,k,d,dist;};
struct Reach{
  Sheet distance,parent;
  array<uint8_t,V> direction,length;
};
struct Problem{
  int n,K,s;
  vector<Ray> rays;
  Sheet below,homes;
  vector<Sheet> static_distance;
  vector<Sheet> common_distance;
  void prepare_batches(){
    common_distance.resize(s);
    for(int p=0;p<s;++p)
      common_distance[p]=search(p,p,below,false).distance;
  }
  void read(){
    cin>>n>>K;s=n*n;assert(s<=V);
    vector<string> field(n);
    for(auto& row:field)cin>>row;
    below.fill(-1);homes.fill(-1);rays.resize(s);
    static constexpr int ry[]={-1,1,0,0},rx[]={0,0,-1,1};
    for(int p=0;p<s;++p){
      int y=p/n,x=p%n;
      char x0=field[y][x];
      if(x0>='a'&&x0<'a'+K)below[p]=x0-'a';
      if(x0>='A'&&x0<'A'+K)homes[x0-'A']=p;
      for(int d=0;d<4;++d){
        rays[p][d].fill(-1);
        if(x0=='#')continue;
        for(int z=1;z<=2;++z){
          int yy=y+z*ry[d],xx=x+z*rx[d];
          if(yy<0||yy>=n||xx<0||xx>=n||field[yy][xx]=='#')break;
          rays[p][d][z-1]=yy*n+xx;
        }
      }
    }
    for(int c=0;c<K;++c)static_distance.push_back(search(homes[c],homes[c],below,false).distance);
  }
  Reach search(int source,int home,const Sheet& ground,bool supports)const{
    Reach results{};
    results.distance.fill(-1);
    Sheet q;q[0]=source;int end=1;
    results.distance[source]=0;
    for(int i=0;i<end;++i){
      int p=q[i];
      if(p==home&&p!=source)continue;
      const int stride=1+int(supports&&ground[p]>=0);
      for(int d=0;d<4;++d)for(int z=0;z<stride;++z){
        int u=rays[p][d][z];
        if(u<0||results.distance[u]>=0)continue;
        results.distance[u]=results.distance[p]+1;
        results.parent[u]=p;results.direction[u]=d;results.length[u]=z+1;
        q[end++]=u;
      }
    }
    return results;
  }
  static void follow(vector<Move>& result,int left,int right,const Reach& reach,const Sheet& board){
    vector<Move> block;
    for(int p=right;p!=left;p=reach.parent[p]){
      const int u=reach.parent[p];
      block.push_back({u,int(board[u]>=0),reach.direction[p],reach.length[p]});
    }
    result.insert(result.end(),block.rbegin(),block.rend());
  }
  vector<Move> solve(bool caravan,Clock::time_point cutoff,int mode=0,int trial=0)const{
    mt19937 rng(720171+trial);
    auto ground=below;
    vector<Move> result;
    while(true){
      int start=-1,best=INT_MIN;
      for(int p=0;p<s;++p)if(ground[p]>=0){
        const int color=ground[p],away=static_distance[color][p];
        int peer[3]={20,20,20},other=0;
        if(mode){
          for(int q=0;q<s;++q)if(q!=p&&ground[q]>=0){
            const int dist=common_distance[p][q];
            assert(dist>=0);
            if(ground[q]==color&&static_distance[color][q]<=away+3){
              if(dist<peer[2]){
                peer[2]=dist;
                if(peer[2]<peer[1])swap(peer[2],peer[1]);
                if(peer[1]<peer[0])swap(peer[1],peer[0]);
              }
            }else if(dist<=2)++other;
          }
        }
        int quality=away*32;
        if(mode){
          const int noise=int(rng()%640);
          quality+=noise;
          const int workload=(max(0,16-peer[0])*2+max(0,12-peer[1])+max(0,8-peer[2]));
          const int potential=(trial%2?17:10)*workload-5*other;
          // Both arms rank all current starts and compute the same features.
          // The randomized control does not apply the batch-aware term.
          if(mode==2)quality+=potential;
        }
        if(quality>best){best=quality;start=p;}
      }
      if(start<0)break;
      if(caravan&&Clock::now()>=cutoff)return {};
      int color=ground[start],home=homes[color],carrying=1;
      int at=start;ground[at]=-1;
      while(true){
        const auto reach=search(at,home,ground,true);
        assert(reach.distance[home]>0);
        int pickup=-1,quality=INT_MIN;
        if(caravan&&carrying<7){
          for(int u=0;u<s;++u)if(ground[u]==color){
            const int d=reach.distance[u];
            if(d<0||d>=reach.distance[home])continue;
            const int gain=12*(reach.distance[home]-d)-3*static_distance[color][u];
            if(gain>quality){quality=gain;pickup=u;}
          }
        }
        if(pickup<0){
          follow(result,at,home,reach,ground);
          break;
        }
        follow(result,at,pickup,reach,ground);
        ++carrying;ground[pickup]=-1;at=pickup;
      }
    }
    assert(!result.empty()&&result.size()<=100000);
    return result;
  }
};
}
int main(){
  auto deadline=Clock::now()+chrono::milliseconds(1420);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();
  auto answer=p.solve(false,deadline);
  const int baseline=answer.size();
  if(AHC072_CARAVAN){
    auto alt=p.solve(true,deadline);
    if(!alt.empty()&&alt.size()<answer.size())answer=move(alt);
  }
  const int frozen=answer.size();
  int trials=0;
  if(AHC072_BATCH){
    p.prepare_batches();
    for(int i=0;i<8&&Clock::now()<deadline;++i){
      auto alt=p.solve(true,deadline,AHC072_BATCH,i);
      if(alt.empty())break;
      if(alt.size()<answer.size())answer=move(alt);
      ++trials;
    }
  }
  cerr<<"base="<<baseline<<" moves="<<answer.size();
  if(AHC072_BATCH)cerr<<" frozen="<<frozen<<" trials="<<trials;
  cerr<<'\n';
  for(auto [v,k,d,h]:answer)cout<<v/p.n<<' '<<v%p.n<<' '<<k<<' '<<directions[d]<<' '<<h<<'\n';
}
