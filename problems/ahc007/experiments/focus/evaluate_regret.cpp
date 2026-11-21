#pragma GCC optimize("O3")
#include <bits/stdc++.h>
using namespace std;
constexpr int N=400,M=1995;
struct Dsu {
  array<int,N> p;
  Dsu(){p.fill(-1);}
  int root(int a) {return p[a]<0?a:p[a]=root(p[a]);}
  bool join(int a,int b) {
    a=root(a);b=root(b); if(a==b)return false;
    if(p[a]>p[b])swap(a,b);p[a]+=p[b];p[b]=a;return true;
  }
};
int main(int argc,char**argv){
  if(argc!=3)return 1;
  ifstream in(argv[1]),out(argv[2]);
  array<int,N> x,y; for(int i=0;i<N;++i)in>>x[i]>>y[i];
  array<int,M> u,v,w,ord;
  for(int i=0;i<M;++i)in>>u[i]>>v[i];
  for(int& z:w)in>>z;
  iota(ord.begin(),ord.end(),0);
  stable_sort(ord.begin(),ord.end(),[&](int a,int b){return w[a]<w[b];});
  Dsu full,tree; int B=0,A=0;
  for(int j:ord)if(full.join(u[j],v[j]))B+=w[j];
  int total=0;
  for(int i=0;i<M;++i) {
    int take=0; if(!(out>>take))return 1;
    if(tree.root(u[i])==tree.root(v[i])) {
      if(take)return 1;
      continue;
    }
    Dsu future=tree;
    int k=-1;
    for(int j:ord)if(j>i && future.join(u[j],v[j]) && future.root(u[i])==future.root(v[i])){
      k=j;break;
    }
    if(k<0 && !take)return 1;
    int loss=k<0?0:max(0,(take?1:-1)*(w[i]-w[k]));
    total+=loss;
    int alternatives=0;
    for(int j=i+1;j<M;++j) {
      int a=tree.root(u[j]),b=tree.root(v[j]);
      int c=tree.root(u[i]),d=tree.root(v[i]);
      alternatives+=(a==c&&b==d)||(a==d&&b==c);
    }
    if(loss)cout<<i<<','<<take<<','<<loss<<','<<k<<','<<alternatives<<'\n';
    if(take)tree.join(u[i],v[i]),A+=w[i];
  }
  if(A-B!=total)return 2;
}
