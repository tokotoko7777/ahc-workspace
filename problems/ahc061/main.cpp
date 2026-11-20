#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>

#if !defined(AHC061_ROLLOUT_TWO) && !defined(AHC061_ROLLOUT_ONE) && \
    !defined(AHC061_ACCESS) && !defined(AHC061_NO_ROLLOUT)
#define AHC061_ROLLOUT_ONE
#endif

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
    State simulate(State state,const std::array<int,8>& moves) const {
        std::array<int,100> count{};
        for(int p=0;p<M;++p) ++count[moves[p]];
        for(int p=0;p<M;++p) {
            const int a=moves[p],o=state.owner[a];
            if(count[a]>1 && o!=p) continue;
            // A defending owner that already processed this cell cannot change its
            // owner; cells with no defending contender are never modified.
            if(o==p) state.level[a]=std::min(U,state.level[a]+1);
            else if(state.level[a]<=1) state.owner[a]=p,state.level[a]=1;
            else {--state.level[a];continue;}
            state.piece[p]=a;
        }
        return state;
    }
    int sample(const Prob& pr,double u,int fallback) const {
        for(int i=0;i<N*N;++i) if((u-=pr[i])<0) return i;
        return fallback;
    }
    double exact_value(const State& state) const {
        const auto score=land(state);
        return std::llround(1e5*std::log2(1.+double(score[0])/
                            *std::max_element(score.begin()+1,score.begin()+M)));
    }
    std::array<int,8> policy2(const State& st,const std::array<double,8>& draw) const {
        std::array<int,8> move{};
        std::array<Prob,8> pr{};
        for(int p=1;p<M;++p) {
            std::array<double,5> mean{};
            for(int i=0;i<profiles;++i)
                for(int j=0;j<5;++j) mean[j]+=weights[p][i]*param[i][j];
            auto list=allowed(st,p);
            double best=-1;
            std::vector<int> ties;
            for(int a:list) {
                const int k=type(st,p,a);
                const double w=k<4 ? V[a]*mean[k] : 0;
                if(w>best) best=w,ties.clear();
                if(w==best) ties.push_back(a);
                pr[p][a]=mean[4]/list.size();
            }
            for(int a:ties) pr[p][a]+=(1-mean[4])/ties.size();
            move[p]=sample(pr[p],draw[p],st.piece[p]);
        }
        auto points=land(st);
        const double top=*std::max_element(points.begin()+1,points.begin()+M);
        double best=-1;
        for(int a:allowed(st,0)) {
            const int p=st.owner[a],lv=st.level[a];
            double safe=1;
            for(int j=1;j<M;++j) safe*=1-pr[j][a];
            double value=p==0 ? (lv<U ? 1. : 0.)+.55*(1-safe)
              : p<0 ? 1.2*safe : (lv==1 ? 1.+points[p]/top : .6+.45*points[p]/top)*safe;
            value*=V[a];
            if(value>best) best=value,move[0]=a;
        }
        return move;
    }
    int rollout(const std::vector<std::pair<double,int>>& ranked,const std::array<Prob,8>& prob,
                int turn,const std::array<int,8>& points,double strongest,bool two_ply,
                std::chrono::steady_clock::time_point deadline) const {
        if(std::chrono::steady_clock::now()>=deadline) return ranked[0].second;
        constexpr int scenarios=6;
        std::array<std::array<double,8>,2*scenarios> draws{};
        uint32_t rng=6774703+99171u*turn;
        for(auto& d:draws) for(int p=1;p<M;++p) {
            rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
            d[p]=(rng+.5)/4294967296.;
        }
        std::array<std::array<int,8>,2*scenarios> root{};
        for(unsigned k=0;k<root.size();++k)
            for(int p=1;p<M;++p) root[k][p]=sample(prob[p],draws[k][p],s.piece[p]);
        const double base=exact_value(s), scale=.6931471805599453*(strongest+points[0])*1e-5;
        double best=-std::numeric_limits<double>::infinity();
        int pick=ranked[0].second;
        for(unsigned j=0;j<std::min<size_t>(6,ranked.size());++j) {
            const auto [heuristic,a]=ranked[j];
            double expected=0;
            for(int k=0;k<scenarios;++k) {
                auto first=root[k],second=root[k+scenarios];
                first[0]=second[0]=a;
                State s1=simulate(s,first);
                const double one=exact_value(s1)-base;
                if(two_ply && turn+1<T) {
                    State s2=simulate(s1,policy2(s1,draws[k+scenarios]));
                    expected+=one+.62*(exact_value(s2)-base-one);
                } else expected+=(one+exact_value(simulate(s,second))-base)*.5;
            }
            double value=heuristic*.85+scale*expected/scenarios;
            if(value>best) best=value,pick=a;
        }
        return pick;
    }
    int choose(int turn,[[maybe_unused]] std::chrono::steady_clock::time_point deadline) {
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
#if defined(AHC061_ROLLOUT_ONE) || defined(AHC061_ROLLOUT_TWO)
        std::vector<std::pair<double,int>> ranked;
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
#if defined(AHC061_ROLLOUT_ONE) || defined(AHC061_ROLLOUT_TWO)
            ranked.emplace_back(value,a);
#endif
        }
#if defined(AHC061_ROLLOUT_ONE) || defined(AHC061_ROLLOUT_TWO)
        std::sort(ranked.begin(),ranked.end(),[](auto a,auto b) {
            return a.first!=b.first ? a.first>b.first : a.second<b.second;
        });
#ifdef AHC061_ROLLOUT_TWO
        pick=rollout(ranked,probs,turn,points,strongest,true,deadline);
#else
        pick=rollout(ranked,probs,turn,points,strongest,false,deadline);
#endif
#endif
        return pick;
    }
    bool feedback(bool trace) {
        std::array<int,8> moves;
        for(int p=0;p<M;++p) {
            int x,y;if(!(std::cin>>x>>y)) return false;
            moves[p]=x*N+y;
            if(p>0) observe(p,moves[p]);
        }
        State expected;
        if(trace) {
            expected=simulate(s,moves);
            std::cerr<<"Turn";
            for(int p=0;p<M;++p) std::cerr<<' '<<moves[p];
            std::cerr<<'\n';
        }
        for(int p=0;p<M;++p) {
            int x,y;std::cin>>x>>y;s.piece[p]=x*N+y;
        }
        for(int i=0;i<N*N;++i) std::cin>>s.owner[i];
        for(int i=0;i<N*N;++i) std::cin>>s.level[i];
        if(trace && std::cin &&
           (s.owner!=expected.owner || s.level!=expected.level || s.piece!=expected.piece)) {
            std::cerr<<"Joint transition mismatch\n";
            std::abort();
        }
        return bool(std::cin);
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(1600);
    Problem p;
    if(!p.init()) return 0;
    const bool trace=std::getenv("AHC061_TRACE")!=nullptr;
    for(int t=0;t<p.T;++t) {
        int a=p.choose(t,deadline);
        std::cout<<a/p.N<<' '<<a%p.N<<std::endl;
        if(!p.feedback(trace)) return 0;
    }
    const auto s=p.land(p.s);
    const int rival=*std::max_element(s.begin()+1,s.begin()+p.M);
    std::cerr<<"Raw score = "<<std::llround(1e5*std::log2(1.+double(s[0])/rival))<<'\n';
}
