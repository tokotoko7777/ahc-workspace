#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>

struct State {
    std::array<int,100> owner{},level{};
    std::array<int,8> piece{};
};
struct Problem {
    int N,M,T,U;
    State s;
    std::array<int,100> V;
    std::array<std::vector<int>,100> adjacent;
    static constexpr int profiles=48;
    std::array<std::array<double,5>,profiles> param;
    using Prob=std::array<double,100>;
    std::array<std::array<double,profiles>,8> weights;
    std::array<std::array<double,profiles>,8> best_value;
    std::array<std::array<int,profiles>,8> best_count;
    std::array<std::vector<int>,8> choices;

    bool init() {
        if(!(std::cin>>N>>M>>T>>U)) return false;
        for(int i=0;i<N*N;++i) {
            std::cin>>V[i];
            for(auto [dx,dy]:std::array<std::pair<int,int>,4>{{{1,0},{-1,0},{0,1},{0,-1}}}) {
                int x=i/N+dx,y=i%N+dy;
                if(x>=0 && x<N && y>=0 && y<N) adjacent[i].push_back(x*N+y);
            }
        }
        s.owner.fill(-1);
        for(int p=0;p<M;++p) {
            int x,y;std::cin>>x>>y;
            s.piece[p]=x*N+y;s.owner[x*N+y]=p;s.level[x*N+y]=1;
            weights[p].fill(1.0/profiles);
        }
        unsigned seed=332997;
        for(auto& model:param) {
            for(int k=0;k<5;++k) {
                seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
                const double u=(seed+.5)/4294967296.;
                model[k]=k<4 ? .3+.7*u : .1+.4*u;
            }
        }
        return true;
    }

    std::vector<int> allowed(const State& cur,int p) const {
        std::array<bool,100> seen{},blocked{};
        for(int i=0;i<M;++i) if(i!=p) blocked[cur.piece[i]]=true;
        std::array<int,100> queue{};
        int head=0,tail=1;
        queue[0]=cur.piece[p];seen[cur.piece[p]]=true;
        std::vector<int> out{cur.piece[p]};
        while(head<tail) {
            const int at=queue[head++];
            for(int j:adjacent[at]) if(!seen[j] && !blocked[j]) {
                out.push_back(j);seen[j]=true;
                if(cur.owner[j]==p) queue[tail++]=j;
            }
        }
        std::sort(out.begin(),out.end()); // deterministic tie behavior
        return out;
    }

    int type(const State& cur,int p,int cell) const {
        int o=cur.owner[cell];
        return o<0 ? 0 : o==p ? (cur.level[cell]<U ? 1 : 4)
                            : cur.level[cell]==1 ? 2 : 3;
    }
    Prob forecast(int p) {
        Prob forecast{};
        double uniform=0;
        for(int k=0;k<profiles;++k) {
            double best=-1;
            std::array<int,100> ties;
            int count=0;
            for(int a:choices[p]) {
                const int kind=type(s,p,a);
                double v=kind<4 ? V[a]*param[k][kind] : 0.;
                if(v>best) best=v,count=0;
                if(v==best) ties[count++]=a;
            }
            best_value[p][k]=best;
            best_count[p][k]=count;
            const double e=param[k][4];
            uniform+=weights[p][k]*e/choices[p].size();
            for(int i=0;i<count;++i) forecast[ties[i]]+=weights[p][k]*(1-e)/count;
        }
        for(int a:choices[p]) forecast[a]+=uniform;
        return forecast;
    }
    void observe(int p,int move) {
        double total=0;
        for(int k=0;k<profiles;++k) {
            const double e=param[k][4];
            const int kind=type(s,p,move);
            const double v=kind<4 ? V[move]*param[k][kind] : 0.;
            double likelihood=e/choices[p].size()+
                (v==best_value[p][k] ? (1-e)/best_count[p][k] : 0.);
            // A broad mixture avoids locking onto one finite-grid approximation.
            total+=weights[p][k]*=likelihood;
        }
        for(double& w:weights[p]) w=.992*w/total+.008/profiles;
    }
    std::array<int,8> land(const State& state) const {
        std::array<int,8> a{};
        for(int i=0;i<N*N;++i) if(state.owner[i]>=0) a[state.owner[i]]+=V[i]*state.level[i];
        return a;
    }
    int choose(int turn) {
        for(int p=0;p<M;++p) choices[p]=allowed(s,p);
        std::array<Prob,8> probs;
        for(int p=1;p<M;++p) probs[p]=forecast(p);
        auto points=land(s);
        double strongest=*std::max_element(points.begin()+1,points.begin()+M);
        const double remaining=double(T-turn-1)/T;
        std::array<double,8> rival{};
        for(int p=1;p<M;++p) rival[p]=.45+.55*points[p]/strongest;
#ifdef AHC061_ACCESS
        auto opportunity=[&](const State& state,int p,const std::vector<int>& list) {
            double out=0;
            for(int c:list) {
                if(state.owner[c]<0) out+=V[c]*.70;
                else if(state.owner[c]!=p && state.level[c]==1) out+=V[c]*.29;
                else if(state.owner[c]==p) out+=V[c]*(U-state.level[c])*.16;
            }
            return out;
        };
        std::array<double,8> base{};
        for(int p=0;p<M;++p) base[p]=opportunity(s,p,choices[p]);
#endif
        int pick=s.piece[0];
        double best=-1e30;
        for(int a:choices[0]) {
            int o=s.owner[a],level=s.level[a];
            const bool capture=o!=0 && level<=1;
            double safe=1,guard=1;
            for(int p=1;p<M;++p) {
                safe*=1-probs[p][a];
                guard*=1-probs[p][a]*rival[p];
            }
            double gain=0;
            if(o<0) gain=(1+remaining*(.45+.17*(U-1)))*safe;
            else if(o==0) gain=(level<U ? 1. : 0.)+(1-guard)*.78;
            else if(capture) gain=(1+(.75+.22*remaining)*rival[o])*safe;
            else gain=(.75+.40*remaining)*rival[o]*safe;
            double value=V[a]*gain;
#ifdef AHC061_ACCESS
            if(capture && remaining>0) {
                State next=s;
                next.owner[a]=0;next.level[a]=1;next.piece[0]=a;
                double delta=opportunity(next,0,allowed(next,0))-base[0];
                // Piece blocking changes adjacency too, so only nearby or previous
                // owners are evaluated for access denial.
                for(int p=1;p<M;++p) {
                    if(std::find(choices[p].begin(),choices[p].end(),a)==choices[p].end() &&
                       std::find(choices[p].begin(),choices[p].end(),s.piece[0])==choices[p].end()) continue;
                    const double diff=opportunity(next,p,allowed(next,p))-base[p];
                    delta-=rival[p]*std::min(15000.,std::max(-15000.,diff));
                }
                value+=remaining*.46*safe*delta;
            }
#endif
            if(value>best) best=value,pick=a;
        }
        return pick;
    }
    bool feedback(bool trace) {
        std::array<int,8> moves;
        for(int p=0;p<M;++p) {
            int x,y;if(!(std::cin>>x>>y)) return false;
            moves[p]=x*N+y;
            if(p>0) observe(p,moves[p]);
        }
        if(trace) {
            std::cerr<<"Turn";
            for(int p=0;p<M;++p) std::cerr<<' '<<moves[p];
            std::cerr<<'\n';
        }
        for(int p=0;p<M;++p) {
            int x,y;std::cin>>x>>y;s.piece[p]=x*N+y;
        }
        for(int i=0;i<N*N;++i) std::cin>>s.owner[i];
        for(int i=0;i<N*N;++i) std::cin>>s.level[i];
        return bool(std::cin);
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Problem p;
    if(!p.init()) return 0;
    const bool trace=std::getenv("AHC061_TRACE")!=nullptr;
    for(int t=0;t<p.T;++t) {
        int a=p.choose(t);
        std::cout<<a/p.N<<' '<<a%p.N<<std::endl;
        if(!p.feedback(trace)) return 0;
    }
    const auto s=p.land(p.s);
    const int rival=*std::max_element(s.begin()+1,s.begin()+p.M);
    std::cerr<<"Raw score = "<<std::llround(1e5*std::log2(1.+double(s[0])/rival))<<'\n';
}
