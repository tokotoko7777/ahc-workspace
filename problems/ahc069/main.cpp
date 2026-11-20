#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#if !defined(AHC069_NO_MOVES) && !defined(AHC069_NO_RESERVE) && !defined(AHC069_RESERVE_RELEASES)
#ifndef AHC069_RESERVE_STATIC
#define AHC069_RESERVE_STATIC
#endif
#endif

using Clock=std::chrono::steady_clock;
using U=uint64_t;
struct Region {
    int orientation=0,row=0,col=0,w=0,h=0,tail=0;
    bool top=false,right=false;
    int length=0;
};
struct Guest {
    int end{},size{},value{},worst{};
    std::vector<int> cells;
};
struct Park {
    int N,M,free=0,capacity=0;
    double observed_len=0,observed_area=0,observed_density=0;
    double R;
    U limit;
    std::array<std::array<U,50>,2> blocked{};
    std::vector<Guest> groups;
    std::priority_queue<std::pair<int,int>,std::vector<std::pair<int,int>>,std::greater<>> ends;
    long long receipts=0;
    int moved=0;
    Clock::time_point deadline=Clock::now()+std::chrono::milliseconds(1580);

    static U bits(int n) { return n ? (U(1)<<n)-1 : 0; }
    void set(int cell,bool value) {
        const int x=cell/N,y=cell%N;
        U bx=U(1)<<x,by=U(1)<<y;
        if(value) blocked[0][x]|=by,blocked[1][y]|=bx;
        else blocked[0][x]&=~by,blocked[1][y]&=~bx;
    }
    int contact(int mode,int x,int y,int w,int h) const {
        if(w==0 || h==0) return 0;
        const auto line=[&](int type,int p,int left,int width) {
            if(p<0 || p>=N) return width;
            return std::popcount((blocked[type][p]>>left)&bits(width));
        };
        return line(mode,x-1,y,w)+line(mode,x+h,y,w)
              +line(1-mode,y-1,x,h)+line(1-mode,y+w,x,h);
    }
    Region find(int P) const {
        Region best;
        int record=1<<30;
        if(free<P) return best;
        int mid=int(std::sqrt(P));
        for(int mode=0;mode<2;++mode) {
            std::array<std::array<U,50>,20> runs{};
            const int hi=std::min(P,mid+3);
            for(int r=0;r<N;++r) {
                U available=limit^blocked[mode][r];
                runs[1][r]=available;
                for(int u=2;u<=hi;++u) runs[u][r]=runs[u-1][r]&(available>>(u-1));
            }
            for(int w=std::max(2,mid-2);w<=hi;++w) {
                int h=(P+w-1)/w,tail=P-w*(h-1);
                const int l=2*(w+h);
                if(best.length && l>best.length) continue;
                for(int x=0;x+h<=N;++x) for(int t=0;t<=(tail!=w);++t) {
                    const int row=x+(t ? 1 : 0),last=t ? x : x+h-1;
                    U run=limit;
                    for(int j=row;j<row+h-1 && run;++j) run&=runs[w][j];
                    if(!run) continue;
                    for(int side=0;side<=(tail!=w);++side) {
                        const int shift=side ? w-tail : 0;
                        for(U fit=run & (runs[tail][last]>>shift);fit;fit&=fit-1) {
                            int y=std::countr_zero(fit);
                            const int value=128*l-contact(mode,row,y,w,h-1)-contact(mode,last,y+shift,tail,1);
                            if(value<record) {
                                record=value;
                                best={mode,x,y,w,h,tail,bool(t),bool(side),l};
                            }
                        }
                    }
                }
            }
        }
        return best;
    }
    std::vector<int> cells(const Region& a) const {
        std::vector<int> out;
        if(!a.length) return out;
        const int ta=a.top ? a.row : a.row+a.h-1;
        for(int r=a.row;r<a.row+a.h;++r) {
            const int len=r==ta ? a.tail : a.w;
            const int offs=r==ta && a.right ? a.w-a.tail : 0;
            for(int c=a.col+offs;c<a.col+offs+len;++c)
                out.push_back(a.orientation==0 ? r*N+c : c*N+r);
        }
        return out;
    }
    void occupancy(const std::vector<int>& cells,bool use) {
        for(int c:cells) set(c,use);
        free+=use ? -int(cells.size()) : int(cells.size());
    }
    long long fee(const Guest& g,int perimeter) const {
        return std::llround(g.value*(4*std::sqrt(g.size)/perimeter));
    }
    long long moving(const Guest& g) const {
        return std::max(1LL,std::llround(g.value*R));
    }
    bool load() {
        if(!(std::cin>>N>>M>>R)) return false;
        groups.resize(M);limit=bits(N);
        for(int i=0;i<N;++i) {
            std::string s;std::cin>>s;
            for(int j=0;j<N;++j) if(s[j]=='#') set(i*N+j,true);
            else ++free;
        }
        capacity=free;
        return true;
    }
    void show(const std::vector<int>& cells) const {
        for(int p:cells) std::cout<<p/N<<' '<<p%N<<'\n';
    }
    bool reserve(int id,int now,int end,int n,int value,bool exact_release) {
        const double duration=end-now;
        const double density=value/(n*duration);
        bool reject=false;
        if(id>=32 && id+24<M) {
            constexpr int slices=6;
            std::array<int,slices> known{};
            const double tail=std::min(duration,double(100000-now));
            double cost=0.;
            const double mean_len=observed_len/id,mean_area=observed_area/id;
            const double rate=std::clamp((id+20.)/(now+2000.),.007,.014);
            for(int j=0;j<id;++j) if(!groups[j].cells.empty()) {
                for(int k=0;k<slices;++k) {
                    const double time=now+(k+.5)*tail/slices;
                    if(groups[j].end>time || !exact_release) known[k]+=groups[j].size;
                }
            }
            for(int k=0;k<slices;++k) {
                const double delta=(k+.5)*tail/slices;
                double future=.84*rate*mean_area*mean_len*(-std::expm1(-delta/mean_len));
                double utilization=(n+known[k]+future)/capacity;
                cost+=std::clamp((utilization-.68)/.51,0.,1.40);
            }
            reject=density<.94*(observed_density/id)*(cost/slices);
        }
        observed_area+=n;
        observed_len+=duration;
        observed_density+=std::min(4.,density);
        return reject;
    }
    void solve() {
        for(int id=0;id<M;++id) {
            int idx,now,end,size,value;
            if(!(std::cin>>idx>>now>>end>>size>>value) || idx!=id) return;
            while(!ends.empty() && ends.top().first<now) {
                int g=ends.top().second;ends.pop();
                occupancy(groups[g].cells,false);
                groups[g].cells.clear();
            }
            Guest& g=groups[id];
            g={end,size,value,0,{}};
            Region place;
            bool hold=false;
#ifdef AHC069_RESERVE_STATIC
            hold=reserve(id,now,end,size,value,false);
#elif defined(AHC069_RESERVE_RELEASES)
            hold=reserve(id,now,end,size,value,true);
#endif
            if(!hold && Clock::now()<deadline) place=find(size);
            int victim=-1,worst=0;
            std::vector<int> relocation;
#ifndef AHC069_NO_MOVES
            if(!hold && !place.length && free>=size && Clock::now()<deadline) {
                std::vector<int> shortlist;
                for(int j=0;j<id;++j) if(!groups[j].cells.empty() && groups[j].end>now+30 &&
                                           moving(groups[j])<value/3)
                    shortlist.push_back(j);
                auto key=[&](int j){return double(moving(groups[j]))/groups[j].size;};
                const int samples=std::min<int>(4,shortlist.size());
                std::partial_sort(shortlist.begin(),shortlist.begin()+samples,shortlist.end(),
                                  [&](int a,int b){return key(a)<key(b);});
                long long record=0;
                for(int k=0;k<samples && Clock::now()<deadline;++k) {
                    int j=shortlist[k];
                    auto& old=groups[j];
                    occupancy(old.cells,false);
                    Region newcomer=find(size);
                    if(newcomer.length) {
                        auto arrived=cells(newcomer);
                        occupancy(arrived,true);
                        Region moved_to=find(old.size);
                        occupancy(arrived,false);
                        if(moved_to.length) {
                            int loss=std::max(old.worst,moved_to.length);
                            long long gain=fee(g,newcomer.length)-moving(old)
                                           -(fee(old,old.worst)-fee(old,loss));
                            if(gain>record) {
                                record=gain;victim=j;place=newcomer;
                                relocation=cells(moved_to);worst=loss;
                            }
                        }
                    }
                    occupancy(old.cells,true);
                }
            }
#endif
            if(victim>=0) {
                auto& old=groups[victim];
                occupancy(old.cells,false);
                occupancy(relocation,true);
                receipts+=fee(old,worst)-fee(old,old.worst)-moving(old);
                old.cells=relocation;old.worst=worst;++moved;
                std::cout<<"1\n"<<victim<<'\n';show(relocation);
            } else std::cout<<"0\n";
            if(place.length) {
                g.worst=place.length;g.cells=cells(place);
                occupancy(g.cells,true);
                ends.push({end,id});receipts+=fee(g,g.worst);
                std::cout<<"Yes\n";show(g.cells);
            } else std::cout<<"No\n";
            std::cout.flush();
        }
        std::cerr<<"Raw score = "<<std::max(0LL,receipts)<<'\n';
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Park p;
    if(p.load()) p.solve();
}
