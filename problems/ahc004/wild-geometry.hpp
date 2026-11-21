#pragma once
#include "anchor-congruence.hpp"
#include "partial-lines.hpp"
namespace ahc004 {
using U=uint64_t;
struct WildAnchor {
    int id[4]{}, row[4]{},phase[4]{};
    int strong=0,weak=0,distinct=0;
    auto key()const{return std::tuple(strong,weak,distinct);}
};
struct WildBank {
    std::vector<PartialLine> patterns;
    int digits[64][40]{};
    U vals[64][20]{}, masks[64][20]{};
    int n=0;
};
inline WildBank make_wild_bank(std::vector<PartialLine> p,int known=12) {
    WildBank b;
    for(auto& s:p)if(s.known>=known && b.n<64){
        b.patterns.push_back(s);
        for(int t=0;t<40;++t)b.digits[b.n][t]=s.letters[t%20]=='.'?8:s.letters[t%20]-'A';
        for(int p=0;p<20;++p)for(int j=0;j<20;++j) {
            const int d=b.digits[b.n][p+j];
            if(d==8)continue;
            b.vals[b.n][p]|=U(d)<<(3*j);
            b.masks[b.n][p]|=U(7)<<(3*j);
        }
        ++b.n;
    }
    return b;
}
inline int wild_signature(int x,int y,int z){return (x*9+y)*9+z;}
inline void wild_mask_table(const WildBank& b,int r,int s,U* strict,U* loose) {
    std::fill_n(strict,729,0);std::fill_n(loose,729,0);
    for(int i=0;i<b.n;++i)for(int p=0;p<20;++p){
        int w[3]={b.digits[i][p],b.digits[i][p+r],b.digits[i][p+s]};
        int known=(w[0]!=8)+(w[1]!=8)+(w[2]!=8);
        if(known==3) {
            strict[wild_signature(w[0],w[1],w[2])]|=U(1)<<i;
            for(int d=0;d<3;++d){
                int a=w[d];w[d]=8;
                loose[wild_signature(w[0],w[1],w[2])]|=U(1)<<i;w[d]=a;
            }
        } else if(known==2) {
            int d=0;while(w[d]!=8)++d;
            for(int a=0;a<9;++a){w[d]=a;loose[wild_signature(w[0],w[1],w[2])]|=U(1)<<i;}
        }
    }
}
struct WildTriples {std::vector<WildAnchor> seeds;int examined=0;bool capped=false;};
inline WildTriples wild_triples(const WildBank& b,int width,int cap,
                              const std::function<bool()>& deadline) {
    WildTriples ret;
    if(b.n<8||width<=0)return ret;
    const auto worse=[](const WildAnchor& a,const WildAnchor& b){return a.key()>b.key();};
    using Heap=std::priority_queue<WildAnchor,std::vector<WildAnchor>,decltype(worse)>;
    // One shortlist per identity triple; otherwise an unrelated high-ranking
    // triple can eliminate every position/phase of the genuine line triple.
    std::vector<std::array<int,3>> ids;
    for(int a=0;a<5;++a)for(int c=a+1;c<5;++c)for(int d=c+1;d<5;++d)ids.push_back({a,c,d});
    std::vector<Heap> pool(ids.size(),Heap(worse));
    for(int r=1;r<20;++r)for(int s=r+1;s<20;++s){
        U strong[729],low[729];wild_mask_table(b,r,s,strong,low);
        for(int t=0;t<int(ids.size());++t) for(int flip=0;flip<2;++flip){
            const auto x=ids[t];int i=x[0],j=x[1+flip],k=x[2-flip];
            const U notused=~((U(1)<<i)|(U(1)<<j)|(U(1)<<k));
            auto& heap=pool[t];
            for(int p=0;p<20;++p)for(int q=0;q<20;++q) {
                if(ret.examined>=cap||((ret.examined&511)==0&&deadline())){ret.capped=true;goto end;}
                ++ret.examined;
                const int minstrong=heap.size()==size_t(width)?heap.top().strong:std::max(2,b.n/5);
                int sc=0,wc=0;U sd=0;
                for(int col=0;col<20;++col) {
                    int z=wild_signature(b.digits[i][col],b.digits[j][p+col],b.digits[k][q+col]);
                    U u=strong[z]&notused, v=low[z]&notused;
                    sc+=bool(u);wc+=bool(u|v);sd|=u;
                    if(sc+19-col<minstrong)break;
                }
                if(sc<minstrong||std::popcount(sd)<minstrong)continue;
                if(heap.size()==size_t(width)&&std::tuple(std::min(sc,std::popcount(sd)),wc,std::popcount(sd))<=heap.top().key())continue;
                std::array<U,20> domain{};
                for(int col=0;col<20;++col)domain[col]=strong[wild_signature(b.digits[i][col],b.digits[j][p+col],b.digits[k][q+col])]&notused;
                int matched=match_columns(domain);
                WildAnchor w{{i,j,k,0},{0,r,s,0},{0,p,q,0},matched,wc,std::popcount(sd)};
                if(matched<minstrong)continue;
                if(heap.size()==size_t(width)){if(w.key()<=heap.top().key())continue;heap.pop();}
                heap.push(w);
            }
        }
    }
end:
    // Round-robin by local rank so a wall-clock cap respects all identities.
    std::vector<std::vector<WildAnchor>> ranked;
    for(auto& h:pool){
        std::vector<WildAnchor> v;
        while(!h.empty()){v.push_back(h.top());h.pop();}
        std::reverse(v.begin(),v.end());ranked.push_back(std::move(v));
    }
    for(int i=0;i<width;++i)for(const auto& v:ranked)if(i<int(v.size()))ret.seeds.push_back(v[i]);
    return ret;
}
static inline int wild_hits(const WildBank& b,const WildAnchor& a,int k,int col,int id,int p){
    int cnt=0;
    for(int t=0;t<k;++t){
        int v=b.digits[id][p+a.row[t]],q=b.digits[a.id[t]][col+a.phase[t]];
        if(v==8||q==8)continue;
        if(v!=q)return -1;
        ++cnt;
    }
    return cnt;
}
struct WildFourth {std::vector<WildAnchor> ranked;int tested=0,parents=0;bool capped=false;};
inline WildFourth wild_fourth(const WildBank& b,const WildTriples& tris,int visit_cap,
                             const std::function<bool()>& deadline) {
    WildFourth ret;
    for(const WildAnchor& t:tris.seeds) {
        if(deadline()){ret.capped=true;break;}
        ++ret.parents;
        U d4[20][20][9]{},d3[20][20][9]{};
        unsigned used_rows=0;U used=0;
        for(int i=0;i<3;++i){used_rows|=1<<t.row[i];used|=U(1)<<t.id[i];}
        for(int c=0;c<20;++c)for(int id=0;id<b.n;++id)if(!(used>>id&1))for(int p=0;p<20;++p){
            const int hits=wild_hits(b,t,3,c,id,p);if(hits<2)continue;
            for(int r=1;r<20;++r)if(!(used_rows>>r&1)){
                const int x=b.digits[id][p+r];
                if(x<8){
                    if(hits==3)d4[r][c][x]|=U(1)<<id;
                    else d3[r][c][x]|=U(1)<<id;
                }else if(hits==3){
                    for(int d=0;d<8;++d)d3[r][c][d]|=U(1)<<id;
                }
                if(hits==3)d3[r][c][8]|=U(1)<<id;
            }
        }
        WildAnchor best=t;
        best.id[3]=-1;best.strong=std::max(2,b.n/7);
        best.weak=-1;best.distinct=-1;
        for(int r=1;r<20;++r)if(!(used_rows>>r&1))
        for(int id=0;id<b.n;++id)if(!(used>>id&1))
        for(int p=0;p<20;++p){
            if(ret.tested>=visit_cap||((ret.tested&1023)==0&&deadline())){ret.capped=true;goto done;}
            ++ret.tested;
            std::array<U,20> mask{},loose{};
            int sc=0,wc=0;U sd=0;
            for(int c=0;c<20;++c){
                int d=b.digits[id][p+c];
                U strict=d4[r][c][d]&~(U(1)<<id);
                mask[c]=strict;
                loose[c]=strict|(d3[r][c][d]&~(U(1)<<id));
                sc+=bool(strict);wc+=bool(loose[c]);sd|=strict;
                if(sc+19-c<best.strong)break;
            }
            if(sc<best.strong||std::popcount(sd)<best.strong)continue;
            int mt=match_columns(mask);
            if(mt<best.strong)continue;
            int wt=match_columns(loose);
            const auto key=std::tuple(mt,wt,wc);
            if(best.id[3]>=0&&key<=best.key())continue;
            best=t;
            best.id[3]=id;best.row[3]=r;best.phase[3]=p;
            best.strong=mt;best.weak=wt;best.distinct=wc;
        }
        if(best.id[3]>=0)ret.ranked.push_back(best);
    }
done:
    std::stable_sort(ret.ranked.begin(),ret.ranked.end(),[](const auto& a,const auto& b){return a.key()>b.key();});
    return ret;
}
struct WildBoard {
    std::vector<std::string> board;
    int rows=0,columns=0,extensions=0;
};
inline WildBoard wild_realize(const WildBank& b,const WildAnchor& w,
                                   const std::function<bool()>& deadline) {
    WildBoard ret{std::vector<std::string>(20,std::string(20,'.')),0,0,0};
    U value[2][20]{},known[2][20]{},used=0;
    unsigned taken[2]{};
    // Compile-time target remains portable; GCC can supply POPCNT if permitted.
    auto matches=[&](int id,int axis,int lane,int phase) {
        U m=b.masks[id][phase],v=b.vals[id][phase];
        U common=known[axis][lane]&m;
        if((v^value[axis][lane])&common)return std::pair(-1,0);
        return std::pair(std::popcount(common)/3,std::popcount(m&~common)/3);
    };
    auto place=[&](int id,int axis,int lane,int phase) {
        if((used>>id&1)||matches(id,axis,lane,phase).first<0)return false;
        for(int j=0;j<20;++j){
            int c=b.digits[id][phase+j];
            if(c!=8) {
                value[axis][lane]|=U(c)<<(3*j);known[axis][lane]|=U(7)<<(3*j);
                value[1-axis][j]|=U(c)<<(3*lane);known[1-axis][j]|=U(7)<<(3*lane);
            }
        }
        used|=U(1)<<id;
        if(!(taken[axis]>>lane&1)){taken[axis]|=1<<lane;if(axis)++ret.columns;else ++ret.rows;}
        return true;
    };
    for(int i=0;i<4;++i)if(!place(w.id[i],0,w.row[i],w.phase[i]))return {};
    for(int round=4;round>=3;--round){
        std::array<U,20> edges{};int phase[20][64];
        for(int j=0;j<20;++j)if(!(taken[1]>>j&1))for(int id=0;id<b.n;++id)if(!(used>>id&1)){
            int best=-1;std::pair<int,int> key{-1,-1};
            for(int p=0;p<20;++p){
                auto k=matches(id,1,j,p);
                if(k.first>=round&&k>key){key=k;best=p;}
            }
            if(best>=0){edges[j]|=U(1)<<id;phase[j][id]=best;}
        }
        std::array<int,20> assign;match_columns(edges,&assign);
        for(int j=0;j<20;++j)if(assign[j]>=0&&!place(assign[j],1,j,phase[j][assign[j]]))return {};
    }
    for(int t=0;t<40 && !deadline();++t){
        std::pair<int,int> best={0,0};int pick=-1,xb=0,yb=0,pb=0;
        for(int id=0;id<b.n;++id)if(!(used>>id&1)){
            for(int x=0;x<2;++x)for(int y=0;y<20;++y){
                if(std::popcount(known[x][y])<15)continue;
                for(int p=0;p<20;++p){
                    const auto z=matches(id,x,y,p);
                    if(z.first>=5&&z.second>0&&z>best){best=z;pick=id;xb=x;yb=y;pb=p;}
                }
            }
        }
        if(pick<0)break;
        place(pick,xb,yb,pb);++ret.extensions;
    }
    for(int r=0;r<20;++r)for(int c=0;c<20;++c)
        if((known[0][r]>>(3*c))&7)ret.board[r][c]='A'+((value[0][r]>>(3*c))&7);
    return ret;
}
inline std::array<int,4> wild_anchor_key(const WildAnchor& q) {
    int anchor=0; for(int i=1;i<4;++i)if(q.id[i]<q.id[anchor])anchor=i;
    std::array<int,4> z{};
    for(int i=0;i<4;++i)
        z[i]=(q.id[i]*20+(q.row[i]-q.row[anchor]+20)%20)*20+(q.phase[i]-q.phase[anchor]+20)%20;
    std::sort(z.begin(),z.end());
    return z;
}
} // namespace ahc004
