#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

constexpr int N=5, C=N*N;
using Input=std::array<std::array<int,N>,N>;
struct Crane {int at=-1, held=-1;};
struct Yard {
    const Input& input;
    std::array<Crane,N> cranes;
    std::array<std::string,N> actions;
    std::array<int,C> at,cell;
    std::array<int,N> read{},next{};
    std::array<bool,C> sent{};
    int turns=0,inversions=0,remaining=C,wrong=0;
    explicit Yard(const Input& a):input(a) {
        at.fill(-1);cell.fill(-1);
        for(int i=0;i<N;++i) cranes[i].at=i*N,next[i]=i*N;
    }
    void feed() {
        for(int r=0;r<N;++r) if(read[r]<N && cell[r*N]<0) {
            bool blocked=false;
            for(const auto& c:cranes) blocked|=c.at==r*N&&c.held>=0;
            if(!blocked) {
                int id=input[r][read[r]++];
                cell[r*N]=id;at[id]=r*N;
            }
        }
    }
    void tick(const std::array<char,N>& commands) {
        feed();
        ++turns;
        for(int i=0;i<N;++i) {
            auto& z=cranes[i];
            char a=commands[i];
            actions[i]+=a;
            if(a=='U') z.at-=N;
            if(a=='D') z.at+=N;
            if(a=='L') --z.at;
            if(a=='R') ++z.at;
            if(a=='B') z.at=-1;
            if(a=='P') {
                z.held=cell[z.at];
                at[z.held]=-1;
                cell[z.at]=-1;
            }
            if(a=='Q') {
                cell[z.at]=z.held;
                at[z.held]=z.at;
                z.held=-1;
            }
        }
        for(int r=0;r<N;++r) {
            int end=r*N+N-1,b=cell[end];
            if(b<0) continue;
            at[b]=-1;cell[end]=-1;--remaining;
            if(b/N!=r) ++wrong;
            else for(int j=b+1;j<(r+1)*N;++j) inversions+=sent[j];
            sent[b]=true;
            while(next[r]<(r+1)*N && sent[next[r]]) ++next[r];
        }
    }
    static int dist(int a,int b) {
        return std::abs(a/N-b/N)+std::abs(a%N-b%N);
    }
    void prefix(int mask,int second) {
        std::array<std::string,N> starts;
        int horizon=1;
        for(int r=0;r<N;++r) {
            if(second>>r&1) starts[r]="PRRQLLPRQL";
            else if(mask>>r&1) starts[r]="PRQL";
            if(r) starts[r]+='B';
            horizon=std::max(horizon,int(starts[r].size()));
        }
        for(int t=0;t<horizon;++t) {
            std::array<char,N> commands;
            for(int i=0;i<N;++i) commands[i]=t<int(starts[i].size())?starts[i][t]:'.';
            tick(commands);
        }
    }
    void one(char c) {tick({c,'.','.','.','.'});}
    void move(int to) {
        while(cranes[0].at/N<to/N) one('D');
        while(cranes[0].at/N>to/N) one('U');
        while(cranes[0].at%N<to%N) one('R');
        while(cranes[0].at%N>to%N) one('L');
    }
    void relocate(int from,int to) {
        move(from);one('P');move(to);one('Q');
    }
    void finish() {
        while(remaining) {
            feed();
            const int from=cranes[0].at;
            int best=-1,priority=std::numeric_limits<int>::max();
            for(int r=0;r<N;++r) {
                const int id=next[r],exit=r*N+N-1;
                if(id==exit+1 || at[id]<0) continue;
                const int cost=dist(from,at[id])+dist(at[id],exit);
                if(cost<priority) priority=cost,best=id;
            }
            if(best>=0) {
                relocate(at[best],best/N*N+N-1);
                continue;
            }
            int src=-1,dst=-1;
            priority=std::numeric_limits<int>::max();
            for(int r=0;r<N;++r) if(cell[r*N]>=0) {
                int urgent=N;
                for(int k=read[r];k<N;++k) {
                    int id=input[r][k];
                    if(id==next[id/N]) urgent=std::min(urgent,k-read[r]);
                }
                if(urgent==N) continue;
                const int id=cell[r*N],exit=id/N*N+N-1;
                for(int q=0;q<C;++q) if(q%N>0 && q%N<N-1 && cell[q]<0) {
                    const int value=14*urgent+3*dist(from,r*N)+4*dist(r*N,q)+3*dist(q,exit);
                    if(value<priority) priority=value,src=r*N,dst=q;
                }
            }
            if(src>=0) {
                relocate(src,dst);
            } else {
                // If every staging slot is occupied, legally open a slot
                // by shipping the smallest visible box to its correct exit.
                for(int b=0;b<C;++b) if(at[b]>=0) {
                    relocate(at[b],b/N*N+N-1);
                    break;
                }
            }
        }
    }
    int score() const {return turns+100*inversions+10000*wrong+1000000*remaining;}
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    if(!(std::cin>>n) || n!=N) return 1;
    Input a;
    for(auto& row:a) for(int& b:row) std::cin>>b;
    Yard initial(a);
    initial.prefix(0,0);initial.finish();
    int best=initial.score();
    auto answer=initial.actions;
#ifndef AHC033_SERIAL_ONLY
    auto test=[&](int first,int second) {
        Yard attempt(a);
        attempt.prefix(first,second);
        attempt.finish();
        if(attempt.score()<best) best=attempt.score(),answer=attempt.actions;
    };
    for(int mask=1;mask<1<<N;++mask) {
        test(mask,0);
        test((1<<N)-1,mask);
    }
#endif
    std::cerr<<"Model = "<<best<<'\n';
    for(auto& line:answer) {
        while(line.size()>1 && line.back()=='.') line.pop_back();
        std::cout<<line<<'\n';
    }
}
