#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <deque>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using Clock=std::chrono::steady_clock;
struct RNG {
    uint32_t x;
    uint32_t next() { x^=x<<13;x^=x>>17;x^=x<<5;return x; }
};
struct State {
    std::array<int,256> food{};
    std::deque<int> body;
    std::vector<int> colors;
};
struct Problem {
    int n,m,c;
    std::array<std::array<int,4>,256> adj;
    std::vector<int> goal;
    State initial;
    static constexpr char letters[]="UDLR";

    bool read() {
        if(!(std::cin>>n>>m>>c)) return false;
        goal.resize(m);
        for(int& a:goal) std::cin>>a;
        for(int i=0;i<n*n;++i) {
            std::cin>>initial.food[i];
            const int x=i/n,y=i%n;
            adj[i]={x>0?i-n:-1,x+1<n?i+n:-1,y>0?i-1:-1,y+1<n?i+1:-1};
        }
        initial.body={4*n,3*n,2*n,n,0};
        initial.colors.assign(5,1);
        return true;
    }
    bool apply(State& s,int d) const {
        const int q=adj[s.body.front()][d];
        if(q<0 || q==s.body[1]) return false;
        s.body.push_front(q);
        if(s.food[q]) {
            s.colors.push_back(std::exchange(s.food[q],0));
        } else s.body.pop_back();
        for(unsigned i=1;i+1<s.body.size();++i) if(s.body[i]==q) {
            for(unsigned j=i+1;j<s.body.size();++j) s.food[s.body[j]]=s.colors[j];
            s.body.resize(i+1);
            s.colors.resize(i+1);
            break;
        }
        return true;
    }
    int score(const State& s,int time) const {
        int e=0;
        for(unsigned k=0;k<s.colors.size();++k) e+=(s.colors[k]!=goal[k]);
        return time+10000*(e+2*(m-int(s.colors.size())));
    }
    std::array<int,256> release(const State& s) const {
        std::array<int,256> r{};
        for(int i=0;i<int(s.body.size());++i) r[s.body[i]]=std::max(r[s.body[i]],int(s.body.size())-i);
        return r;
    }
    std::pair<std::vector<int>,bool> static_path(const State& s,int wanted,RNG& rand) const {
        std::array<int,256> prev,action,dist;
        prev.fill(-2);
        std::array<int,256> q{};
        auto free_at=release(s);
        int root=s.body.front(),begin=0,end=1,pick=-1,best=1<<30;
        prev[root]=-1;q[0]=root;dist[root]=0;
        const int rot=rand.next()%4;
        while(begin<end) {
            int at=q[begin++];
            for(int i=0;i<4;++i) {
                int d=(i+rot)%4,b=adj[at][d];
                if(b<0 || prev[b]!=-2 || free_at[b]>1 || (at==root && b==s.body[1])) continue;
                prev[b]=at;action[b]=d;dist[b]=dist[at]+1;
                if(s.food[b]) {
                    int value=8*dist[b]+int(rand.next()%7)+(s.food[b]==wanted ? 0 : 10000);
                    if(value<best) pick=b,best=value;
                } else q[end++]=b;
            }
        }
        if(pick<0) return {};
        bool correct=s.food[pick]==wanted;
        std::vector<int> path;
        for(int b=pick;prev[b]>=0;b=prev[b]) path.push_back(action[b]);
        std::reverse(path.begin(),path.end());
        return {path,correct};
    }
    std::vector<int> moving_path(const State& s,int wanted,Clock::time_point deadline) const {
        const auto free_at=release(s);
        const int size=n*n,head=s.body.front();
        constexpr int horizon=15;
        std::vector<int> prev((horizon+1)*size*4,-2);
        std::vector<int> q;
        q.reserve(prev.size());
        int attempted=0;
        const auto id=[&](int time,int at,int d){return (time*size+at)*4+d;};
        auto add=[&](int b,int d,int t,int from) {
            if(b<0 || free_at[b]>t || (s.food[b] && s.food[b]!=wanted)) return;
            int p=id(t,b,d);
            if(prev[p]!=-2) return;
            prev[p]=from;q.push_back(p);
        };
        for(int d=0;d<4;++d) if(adj[head][d]!=s.body[1]) add(adj[head][d],d,1,-1);
        for(unsigned at=0;at<q.size();++at) {
            const int p=q[at],b=(p/4)%size,d=p%4,t=(p/4)/size;
            if(at%256==0 && Clock::now()>=deadline) break;
            if(s.food[b]) {
                if(++attempted>18) break;
                std::vector<int> path;
                for(int x=p;x>=0;x=prev[x]) path.push_back(x%4);
                std::reverse(path.begin(),path.end());
                // This compressed BFS represents the initial tail releases but
                // not visits emitted earlier along its candidate route.
                State test=s;
                bool good=true;
                for(unsigned i=0;i<path.size() && good;++i)
                    good=apply(test,path[i]) && (test.colors.size()==s.colors.size()+(i+1==path.size()));
                if(good && test.colors.back()==wanted) return path;
            } else if(t<horizon) {
                for(int e=0;e<4;++e) if((d^1)!=e) add(adj[b][e],e,t+1,p);
            }
        }
        return {};
    }
    std::pair<int,std::string> solve() const {
        const auto deadline=Clock::now()+std::chrono::milliseconds(1600);
        int best=score(initial,0);
        std::string incumbent;
        for(int run=0;run<12 && Clock::now()<deadline;++run) {
            State s=initial;
            RNG rnd{911831u+138083u*unsigned(run)};
            std::string ops;
            int stall=0,best_run=score(s,0);
            while(int(ops.size())<1700 && stall<180 && int(s.colors.size())<m && Clock::now()<deadline) {
                const int wanted=goal[s.colors.size()];
                auto [path,matched]=static_path(s,wanted,rnd);
#ifndef AHC063_STATIC
                if(!matched) {
                    auto moving=moving_path(s,wanted,deadline);
                    if(!moving.empty()) path=std::move(moving);
                }
#endif
                if(path.empty()) {
                    int move=-1,value=1<<30;
                    for(int d=0;d<4;++d) {
                        State next=s;
                        if(!apply(next,d)) continue;
                        int v=score(next,0)+int(rnd.next()%2000);
                        if(v<value) value=v,move=d;
                    }
                    if(move<0) break;
                    path.push_back(move);
                }
                for(int d:path) {
                    if(!apply(s,d)) break;
                    ops.push_back(letters[d]);++stall;
                    int now=score(s,ops.size());
                    if(now<best) best=now,incumbent=ops;
                    if(now<best_run) best_run=now,stall=0;
                }
            }
        }
        return {best,incumbent};
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Problem p;
    if(!p.read()) return 0;
    const auto [value,ops]=p.solve();
    for(char d:ops) std::cout<<d<<'\n';
    std::cerr<<"Cost = "<<value<<'\n';
}
