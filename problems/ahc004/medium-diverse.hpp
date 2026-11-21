#pragma once
#include "wild-geometry.hpp"
#include <cmath>

namespace ahc004 {
inline int wild_match(const U* domains) {
  int mate[64];
  std::fill_n(mate,64,-1);
  int result=0;
  for(int lane=0;lane<20;++lane) {
    U seen=0;
    const auto augment=[&](const auto& self,int col)->bool{
      while(U options=domains[col]&~seen){
        const int id=std::countr_zero(options);
        seen|=U(1)<<id;
        if(mate[id]<0 || self(self,mate[id])) {
          mate[id]=col;
          return true;
        }
      }
      return false;
    };
    result+=augment(augment,lane);
  }
  return result;
}

inline WildFourth medium_fourth(const WildBank& b,const WildTriples& tris,int visit_cap,
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
            U mask[20],loose[20];
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
            int mt=wild_match(mask);
            if(mt<best.strong)continue;
            if(mt==best.strong && wc<best.weak)continue;
            int wt=wild_match(loose);
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

// Keep a small beam at every relative row-offset pair, scored against its
// own phase null. Phases of sparse anchors must not compete only on raw count
// with relative positions that happen to contain fewer wildcard intersections.
inline WildTriples wild_offset_diverse(const WildBank& b, int width, int cap,
                                       const std::function<bool()>& deadline) {
    WildTriples result;
    if(b.n<24 || width<=0)return result;
    struct Ranked {
        WildAnchor a;
        double z=0;
        auto key()const{return std::tuple(z,a.strong,a.distinct,a.weak);}
    };
    std::array<int,3> ids[10]{};int ni=0;
    for(int a=0;a<5;++a)for(int d=a+1;d<5;++d)for(int c=d+1;c<5;++c)ids[ni++]={a,d,c};
    std::vector<Ranked> local[10];
    // Fixed shuffle spreads deadline-limited exploration across offset pairs.
    std::vector<std::pair<int,int>> lanes;
    for(int r=1;r<20;++r)for(int s=r+1;s<20;++s)lanes.emplace_back(r,s);
    uint32_t rng=365823741;
    for(int i=int(lanes.size())-1;i>0;--i){
        rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
        std::swap(lanes[i],lanes[rng%(i+1)]);
    }
    for(auto [r,s]:lanes){
        U strong[729],loose[729];wild_mask_table(b,r,s,strong,loose);
        for(int t=0;t<ni;++t)for(int f=0;f<2;++f){
            const int a=ids[t][0],d=ids[t][1+f],c=ids[t][2-f];
            const U unused=~((U(1)<<a)|(U(1)<<d)|(U(1)<<c));
            int x[20][20],y[20][20];
            for(int p=0;p<20;++p)for(int col=0;col<20;++col){
                x[p][col]=(b.digits[a][col]*9+b.digits[d][p+col])*9;
                y[p][col]=b.digits[c][p+col];
            }
            struct Phase{int u=-1,eligible=-1,distinct=-1,p=0,q=0;};
            Phase hi[3];
            int count=0,sum=0,squares=0;
            for(int p=0;p<20;++p)for(int q=0;q<20;++q){
                if(result.examined>=cap||((result.examined&511)==0&&deadline())){
                    result.capped=true;goto done;
                }
                ++result.examined;
                int nc=0;U all=0;
                for(int col=0;col<20;++col){
                    const U v=strong[x[p][col]+y[q][col]]&unused;
                    nc+=bool(v);all|=v;
                }
                ++count;sum+=nc;squares+=nc*nc;
                if(nc<hi[2].u)continue;
                int nd=std::popcount(all), u=std::min(nc,nd);
                auto key=std::tuple(u,nc,nd);
                if(key<=std::tuple(hi[2].u,hi[2].eligible,hi[2].distinct))continue;
                for(int i=0;i<3;++i)if(key>std::tuple(hi[i].u,hi[i].eligible,hi[i].distinct)){
                    for(int j=2;j>i;--j)hi[j]=hi[j-1];
                    hi[i]={u,nc,nd,p,q};break;
                }
            }
            const double mean=double(sum)/count;
            const double sd=std::sqrt(std::max(.25,double(squares)/count-mean*mean))+.35;
            for(auto h:hi) {
                if(h.u<std::max(2,b.n/6))continue;
                U domains[20];int wc=0;
                for(int col=0;col<20;++col){
                    int q=x[h.p][col]+y[h.q][col];
                    domains[col]=strong[q]&unused;
                    wc+=bool(domains[col]|(loose[q]&unused));
                }
                int mt=wild_match(domains);
                if(mt<std::max(2,b.n/6))continue;
                WildAnchor q{{a,d,c,0},{0,r,s,0},{0,h.p,h.q,0},mt,wc,h.distinct};
                local[t].push_back({q,(mt-mean)/sd});
            }
        }
    }
done:
    // Alternate relative surprise and raw support, within each identity
    // triple. Keep hypotheses distinct even when they rank highly both ways.
    std::vector<WildAnchor> shortlists[10];
    for(int t=0;t<ni;++t){
        auto by_raw=local[t];
        std::stable_sort(local[t].begin(),local[t].end(),[](const auto& a,const auto& b){return a.key()>b.key();});
        std::stable_sort(by_raw.begin(),by_raw.end(),[](const auto& a,const auto& b){return a.a.key()>b.a.key();});
        std::set<std::array<int,4>> recorded;
        for(int a=0;a<int(local[t].size())&&int(shortlists[t].size())<width;++a){
            for(const auto* v:{&local[t],&by_raw}) {
                if(int(shortlists[t].size())>=width)break;
                const auto& q=(*v)[a].a;
                std::array<int,4> key{q.row[1],q.row[2],q.phase[1],q.phase[2]};
                // Record ordered relative rows together with their actual IDs.
                if(q.id[1]>q.id[2]){std::swap(key[0],key[1]);std::swap(key[2],key[3]);}
                if(recorded.insert(key).second)shortlists[t].push_back(q);
            }
        }
    }
    for(int i=0;i<width;++i)for(int t=0;t<ni;++t)
        if(i<int(shortlists[t].size()))result.seeds.push_back(shortlists[t][i]);
    return result;
}
} // namespace ahc004
