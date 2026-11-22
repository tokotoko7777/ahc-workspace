#pragma GCC optimize("O3")
#include <bits/stdc++.h>
#include "cat-target-filter.hpp"
using namespace std;
constexpr int S=30,V=900,I=2000;
array<array<int,4>,V> nb;
int md(int p,int q){return abs(p/S-q/S)+abs(p%S-q%S);}
int dir(char c){return string_view("UDLR").find(toupper(c));}
using Grid=array<unsigned char,V>;
array<int,V> bfs(const Grid& w,int source) {
 array<int,V>d;d.fill(I);int q[V],n=1;q[0]=source;d[source]=0;
 for(int i=0;i<n;i++)for(int v:nb[q[i]])if(v>=0&&!w[v]&&d[v]==I)d[v]=d[q[i]]+1,q[n++]=v;
 return d;
}
struct Cat {int id;CatTargetFilter<V> filter;int support=0,near=0,ready=0,eta=0,legal=0,compact=0,terminal=0,p25=0,p50=0,p75=0,p25ready=0,p25eta=0,p25legal=0,p25later=0;
 int firstLegal=-1,firstP25=-1,firstSupported=-1; double maxP=0.;vector<tuple<int,int,int>> alerts;};
struct Geometry {int before,after,a,b,step,len;};
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 for(int p=0;p<V;p++)nb[p]={p>=S?p-S:-1,p+S<V?p+S:-1,p%S?p-1:-1,p%S+1<S?p+1:-1};
 int n,m,r,c,t;cin>>n;
 vector<int> ps(n),typ(n);vector<Cat> cats;
 for(int j=0;j<n;j++){cin>>r>>c>>t;ps[j]=(r-1)*S+c-1;typ[j]=t;if(t==5)cats.push_back(Cat{j});}
 cin>>m;vector<int> hs(m);
 for(int& p:hs){cin>>r>>c;p=(r-1)*S+c-1;}
 Grid w{};int geom=0,st=0,one=0,lg=0;ShortestFirstMasks<V> shortest;
 vector<Geometry> shapes;
 for(int p=0;p<V;p++)for(int step:{1,S})for(int z=3;z<=12;z++){
  int end=p+(z+3)*step;if(end>=V||(step==1&&p/S!=end/S))continue;
  shapes.push_back({p,end,p+step,end-step,step,z});
 }
 for(int turn=0;turn<=300;turn++){
  Grid forbid{},hum{};
  for(int p:ps){forbid[p]=1;for(int q:nb[p])if(q>=0)forbid[q]=1;}
  for(int p:hs)forbid[p]=1,hum[p]=1;
  array<int,V> group;group.fill(-1);
  vector<int> gs,gh;
  for(int p=0;p<V;p++)if(!w[p]&&group[p]<0){
   int q[V],n=1;q[0]=p;group[p]=gs.size();
   for(int i=0;i<n;i++)for(int v:nb[q[i]])if(v>=0&&!w[v]&&group[v]<0)group[v]=group[p],q[n++]=v;
   gs.push_back(n);gh.push_back(0);
  }
  for(int p:hs)++gh[group[p]];
  for(auto &ct:cats){
   int g=group[ps[ct.id]];
   if(!gh[g]&&gs[g]<=12){ct.compact=1;if(turn==300)ct.terminal=1;}
  }
  if(turn<300)for(auto [b0,b1,a,b,s,len]:shapes){
   if(w[b0]||w[b1]||w[a]||w[b])continue;
   int d1=s==1?0:2,d2=d1+1;
   bool ok=true, humanInside=false;
   for(int q=a;q<=b;q+=s){
    if(w[q]||(nb[q][d1]>=0&&!w[nb[q][d1]])||(nb[q][d2]>=0&&!w[nb[q][d2]])){ok=false;break;}
    if(q!=a&&q!=b&&hum[q])humanInside=true;
   }
   if(!ok)continue;
   ++geom;
   bool ready=hum[b0]&&hum[b1], soon=false;
   if(!humanInside)for(int i=0;i<m;i++)if(md(hs[i],b0)<=1)for(int j=0;j<m;j++)if(i!=j&&md(hs[j],b1)<=1)soon=true;
   st+=ready;one+=soon;
   bool legal=ready&&!humanInside&&!forbid[a]&&!forbid[b];
   lg+=legal;
   for(auto &cat:cats){
    int p=ps[cat.id],g=group[p];
    if(!gh[g])continue;
    bool interior=p>a&&p<b&&(p-a)%s==0;
    if(interior){cat.support=1;cat.ready|=ready;cat.eta|=soon;
      if(cat.firstSupported<0)cat.firstSupported=turn;
      if(legal){cat.legal=1;if(cat.firstLegal<0)cat.firstLegal=turn;
        for(auto [tt,aa,bb]:cat.alerts)if(aa==a&&bb==b&&tt<turn)cat.p25later=1;
      }
      continue;
    }
    if(humanInside||(!soon&&md(p,b0)>5&&md(p,b1)>5))continue;
    if(group[b0]!=g||(md(p,b0)>5&&md(p,b1)>5))continue;
    cat.near=1;
    auto dx=bfs(w,p);
    if(min(dx[a],dx[b])>6)continue;
    auto da=bfs(w,a),db=bfs(w,b);
    Grid w2=w;for(int q=a+s;q<b;q+=s)w2[q]=1;
    auto avoid=bfs(w2,p);
    int available=0; double reset=cat.filter.reset,prob=0.;
    for(int z=0;z<V;z++)if(dx[z]==I||z==p)reset+=cat.filter.target[z];else ++available;
    if(!available)continue;
    const auto term=reset/available;
    for(int z=0;z<V;z++){
     if(dx[z]==I||z==p||(z>=a&&z<=b&&(z-a)%s==0))continue;
     if(avoid[z]<=dx[z])continue; // a route avoiding the interior ties preferred path
     if(dx[z]!=dx[a]+len+1+db[z]&&dx[z]!=dx[b]+len+1+da[z])continue;
     prob+=cat.filter.target[z]+term;
    }
    cat.maxP=max(cat.maxP,prob);
    if(prob>=.25){cat.p25=1;cat.p25ready|=ready;cat.p25eta|=soon;cat.p25legal|=legal;
      if(soon)cat.alerts.emplace_back(turn,a,b);
      if(cat.firstP25<0)cat.firstP25=turn;
    }
    cat.p50|=prob>=.5;cat.p75|=prob>=.75;
   }
  }
  if(turn==300)break;
  string act;cin>>act;
  for(int i=0;i<m;i++)if(act[i]!='.'){
    int q=nb[hs[i]][dir(act[i])];assert(q>=0);
    if(islower(act[i]))w[q]=1;else hs[i]=q;
  }
  int k=0;
  for(int j=0;j<n;j++){
   string obs;cin>>obs;int p=ps[j],mid=-1,fd=-1;
   for(char ch:obs)if(ch!='.'){int d=dir(ch);ps[j]=nb[ps[j]][d];assert(ps[j]>=0&&!w[ps[j]]);if(mid<0)mid=ps[j],fd=d;}
   if(typ[j]==5)cats[k++].filter.observe(p,fd,mid,ps[j],nb,[&](int q){return !w[q];},shortest);
  }
 }
 cout<<"G,"<<geom<<","<<st<<","<<one<<","<<lg<<"\n";
 for(auto &c:cats)cout<<c.id<<","<<c.support<<","<<c.near<<","<<c.ready<<","<<c.eta<<","<<c.legal<<","<<c.compact<<","<<c.terminal<<","<<c.p25<<","<<c.p50<<","<<c.p75<<","<<c.p25ready<<","<<c.p25eta<<","<<c.p25legal<<","<<c.p25later<<","<<c.maxP<<","<<c.firstLegal<<","<<c.firstP25<<","<<c.firstSupported<<"\n";
}
