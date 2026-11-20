#pragma GCC optimize("O3")
#include <bits/stdc++.h>
using namespace std;
using Board=vector<string>;
struct Dsu{
 vector<int> p;
 explicit Dsu(int n):p(n){iota(p.begin(),p.end(),0);}
 int root(int a){return p[a]==a?a:p[a]=root(p[a]);}
 void join(int a,int b){p[root(a)]=root(b);}
};
struct Lit { int id; bool b; };
using Clause=vector<Lit>;
struct Read { int weight=0; vector<Clause> clauses; };
int main(int argc,char** argv){
 if(argc!=3) return 2;
 int n,m; cin>>n>>m;
 map<string,int> freq;
 for(int i=0;i<m;++i){string w;cin>>w;++freq[w];}
 Board a(n),b(n),z(n,string(n,'.'));
 ifstream fa(argv[1]),fb(argv[2]);
 for(int i=0;i<n;++i) if(!(fa>>a[i]&&fb>>b[i])||a[i].size()!=size_t(n)||b[i].size()!=size_t(n)) return 3;
 int best_dist=n*n+1,raw_dist=0,tx=0,ty=0,tr=0;
 for(int r=0;r<n;++r)for(int c=0;c<n;++c) raw_dist+=a[r][c]!=b[r][c];
 for(int t=0;t<2;++t)for(int y=0;y<n;++y)for(int x=0;x<n;++x){
  int dist=0;
  for(int r=0;r<n;++r)for(int c=0;c<n;++c) dist+= a[r][c]!=(t?b[(c+x)%n][(r+y)%n]:b[(r+y)%n][(c+x)%n]);
  if(dist>=best_dist)continue;
  tx=x;ty=y;tr=t;best_dist=dist;
  for(int r=0;r<n;++r)for(int c=0;c<n;++c)z[r][c]=t?b[(c+x)%n][(r+y)%n]:b[(r+y)%n][(c+x)%n];
 }
 b=move(z);
 vector<int> var(n*n,-1),cells;
 for(int r=0;r<n;++r)for(int c=0;c<n;++c)if(a[r][c]!=b[r][c]){
  var[r*n+c]=cells.size();cells.push_back(r*n+c);
 }
 int v=cells.size(), constant=0, missing=0;
 Dsu uf(v);
 vector<Read> reads;
 for(auto& [word,weight]:freq){
  Read read{weight,{}};
  bool fixed=false;
  for(int d=0;d<2&&!fixed;++d)for(int rr=0;rr<n&&!fixed;++rr)for(int cc=0;cc<n&&!fixed;++cc){
   bool ok=true; array<Lit,20> picks; int len=0;
   int r=rr,c=cc;
   for(char t:word){
    bool ba=a[r][c]==t, bb=b[r][c]==t;
    if(!ba&&!bb){ok=false;break;}
    if(ba!=bb)picks[len++]={var[r*n+c],bb};
    if(d==0){if(++c==n)c=0;}else if(++r==n)r=0;
   }
   if(!ok)continue;
   if(len==0){fixed=true;break;}
   read.clauses.emplace_back(picks.begin(),picks.begin()+len);
  }
  if(fixed){constant+=weight;continue;}
  if(read.clauses.empty()){missing+=weight;continue;}
  int first=read.clauses.front().front().id;
  for(auto& clause:read.clauses)for(auto lit:clause)uf.join(first,lit.id);
  reads.push_back(move(read));
 }
 vector<vector<int>> groups(v), scoped(v);
 for(int i=0;i<v;++i)groups[uf.root(i)].push_back(i);
 for(int i=0;i<int(reads.size());++i)scoped[uf.root(reads[i].clauses[0][0].id)].push_back(i);
 int coverage=constant, exact=0, big=0; uint64_t work=0;
 vector<int> sizes,pos(v);
 for(int g=0;g<v;++g) if(!groups[g].empty()){
  auto& ids=groups[g]; auto& rr=scoped[g];
  int k=ids.size(); sizes.push_back(k);
  for(int j=0;j<k;++j) pos[ids[j]]=j;
  int aw=0,bw=0;
  for(int i:rr){
   bool aa=false,bb=false;
   for(auto& c:reads[i].clauses){
    aa|=ranges::none_of(c,[](Lit x){return x.b;});
    bb|=ranges::all_of(c,[](Lit x){return x.b;});
   }
   aw+=aa*reads[i].weight; bw+=bb*reads[i].weight;
  }
  int ae=0,be=0;
  for(int id:ids){int i=cells[id];ae+=a[i/n][i%n]=='.';be+=b[i/n][i%n]=='.';}
  pair<int,int> best=max(pair{aw,ae},pair{bw,be});
  bool whole_b=pair{bw,be}>pair{aw,ae};
  uint32_t chosen=0, empty_a=0,empty_b=0;
  uint64_t clauses=0;
  for(int i:rr) clauses+=reads[i].clauses.size();
  const bool do_exact=k<=16&&work+(max<uint64_t>(1,clauses)<<k)<=1000000;
  if(do_exact) {
   ++exact;
   work+=(max<uint64_t>(1,clauses)<<k);
   vector<vector<pair<uint32_t,uint32_t>>> cased;
   for(int i:rr){
    vector<pair<uint32_t,uint32_t>> opts;
    for(auto& clause:reads[i].clauses){
     uint32_t needA=0,needB=0;
     for(Lit p:clause)(p.b?needB:needA)|=1u<<pos[p.id];
     opts.emplace_back(needA,needB);
    }
    cased.push_back(move(opts));
   }
   uint32_t end=1u<<k;
   chosen=whole_b?end-1:0;
   for(int j=0;j<k;++j){int cell=cells[ids[j]];if(a[cell/n][cell%n]=='.')empty_a|=1u<<j;if(b[cell/n][cell%n]=='.')empty_b|=1u<<j;}
   for(uint32_t x=0;x<end;++x){
    int count=0;
    for(size_t q=0;q<rr.size();++q)for(auto [aa,bb]:cased[q])
     if(!(x&aa)&&!(~x&bb)){count+=reads[rr[q]].weight;break;}
    pair<int,int> score{count,popcount(~x&empty_a)+popcount(x&empty_b)};
    if(score>best){best=score;chosen=x;}
   }
  } else ++big;
  coverage+=best.first;
  for(int j=0;j<k;++j)if(do_exact?bool(chosen>>j&1):whole_b){
   int cell=cells[ids[j]];
   a[cell/n][cell%n]=b[cell/n][cell%n];
  }
 }
 for(auto& r:a)cout<<r<<'\n';
 sort(sizes.rbegin(),sizes.rend());
 cerr<<"raw="<<raw_dist<<" aligned="<<best_dist<<" t="<<tr<<" x="<<tx<<" y="<<ty
     <<" exact="<<exact<<" fallback="<<big<<" constants="<<constant<<" impossible="<<missing
     <<" predicted="<<coverage<<" work="<<work<<" sizes=";
 for(int s:sizes)cerr<<s<<';';
 cerr<<'\n';
}
