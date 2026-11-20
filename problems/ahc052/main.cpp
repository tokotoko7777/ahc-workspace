#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

static constexpr int Cells=900,Robots=10,Keys=10,Depth=3;
static constexpr std::array<int,4> reverse_dir{1,0,3,2};
static constexpr char Symbol[]="UDLRS";
using Clock=std::chrono::steady_clock;
using Position=std::array<int,Robots>;
using Controller=std::array<std::array<int,Robots>,Keys>;

uint64_t scramble(uint64_t x) {
    x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;
    x=(x^(x>>27))*0x94d049bb133111ebULL;
    return x^(x>>31);
}
struct State {
    Position pos{};
    std::bitset<Cells> seen;
    uint64_t signature=0;
    int gained=0;
    int neighborhood=0;
    std::array<int,Depth> commands{};
};
struct Solution {
    Controller keys{};
    std::vector<int> route;
};

struct Problem {
    int n,m,k;
    Position start{};
    std::vector<std::array<int,4>> grid;
    std::vector<uint64_t> token;
    bool read() {
        if(!(std::cin>>n>>m>>k) || n*n>Cells || m!=Robots || k!=Keys) return false;
        grid.resize(n*n);
        token.resize(n*n);
        for(int v=0;v<n*n;++v) {
            grid[v].fill(v);
            token[v]=scramble(0x9e3779b97f4a7c15ULL+v);
        }
        for(int i=0,r,c;i<m;++i) {std::cin>>r>>c;start[i]=r*n+c;}
        for(int r=0;r<n;++r) {
            std::string row;std::cin>>row;
            for(int c=0;c+1<n;++c) if(row[c]=='0') grid[r*n+c][3]=r*n+c+1,grid[r*n+c+1][2]=r*n+c;
        }
        for(int r=0;r+1<n;++r) {
            std::string row;std::cin>>row;
            for(int c=0;c<n;++c) if(row[c]=='0') grid[r*n+c][1]=(r+1)*n+c,grid[(r+1)*n+c][0]=r*n+c;
        }
        return true;
    }
    Controller layouts(int mode,bool different) const {
        Controller a{};
        for(int button=0;button<k;++button) a[button].fill(button%4);
        if(!different) return a;
        constexpr int compass[]{0,3,1,2};
        for(int i=0;i<m;++i) {
            int r=start[i]/n,c=start[i]%n;
            int rotation=((r>=n/2)?2:0)^((c>=n/2)?1:0);
            if(mode>0) rotation=scramble(912324ULL*(mode+9)+43233ULL*i)%4;
            for(int j=0;j<4;++j) a[j+4][i]=compass[(rotation+j)%4];
            for(int j=8;j<10;++j) a[j][i]=scramble(9173ULL*(mode+1)+1153ULL*i+5893ULL*j)%5;
        }
        return a;
    }
    State initial() const {
        State s;
        s.pos=start;
        for(int v:start) if(!s.seen[v]) {
            s.seen.set(v);s.signature^=token[v];++s.gained;
        }
        return s;
    }
    bool move(State& s,const Controller& c,int button) const {
        bool any=false;
        for(int i=0;i<m;++i) {
            int p=s.pos[i];
            int direction=c[button][i];
            int v=(direction==4) ? p : grid[p][direction];
            any|=p!=v;
            s.pos[i]=v;
            if(!s.seen[v]) s.seen.set(v),++s.gained,s.signature^=token[v];
        }
        return any;
    }
    int fallback_dir(const State& s,int mode) const {
        std::vector<int> dist(n*n,-1),action(n*n,-1),q;
        for(int i=0;i<n*n;++i) if(!s.seen[i]) {
            dist[i]=0;
            q.push_back(i);
        }
        for(int j=0;j<int(q.size());++j) for(int step=0;step<4;++step) {
            int direction=(step+mode)%4;
            int p=q[j],v=grid[p][direction];
            if(dist[v]<0) dist[v]=dist[p]+1,action[v]=reverse_dir[direction],q.push_back(v);
        }
        int best=-1;
        for(int p:s.pos) if(best<0 || dist[p]<dist[best]) best=p;
        return action[best];
    }
    std::vector<int> find_actions(const State& current,const Controller& config,
                                  const std::vector<int>& keys,int mode) const {
        std::vector<State> beam{current},next;
        const int width=std::max(7,120/int(keys.size()));
        double highest=0.;
        std::vector<int> chosen;
        for(int depth=0;depth<Depth && !beam.empty();++depth) {
            next.clear();
            std::unordered_set<uint64_t> checked;
            checked.reserve(2*width*keys.size());
            for(const State& base:beam) for(int key:keys) {
                State node=base;
                if(!move(node,config,key)) continue;
                uint64_t hash=node.signature;
                node.neighborhood=0;
                for(int i=0;i<m;++i) {
                    int v=node.pos[i];
                    hash^=scramble(8921ULL*(i+300)+v);
                    for(int u:grid[v]) node.neighborhood+=!node.seen[u];
                }
                if(!checked.insert(hash).second) continue;
                node.commands[depth]=key;
                const int gain=node.gained-current.gained;
                const double score=gain/(depth+1.75)+.0001*node.neighborhood;
                if(gain>0 && score>highest) {
                    highest=score;
                    chosen.assign(node.commands.begin(),node.commands.begin()+depth+1);
                }
                next.push_back(std::move(node));
            }
            const auto cmp=[&](const State& a,const State& b) {
                return std::tuple(a.gained,a.neighborhood,scramble(a.signature+mode))>
                       std::tuple(b.gained,b.neighborhood,scramble(b.signature+mode));
            };
            if(int(next.size())>width) {
                std::nth_element(next.begin(),next.begin()+width,next.end(),cmp);
                next.resize(width);
            }
            beam.swap(next);
        }
        return chosen;
    }
    Solution dfs() const {
        Solution result{layouts(0,false),{}};
        State s=initial();
        std::vector<unsigned char> visited(n*n);
        auto browse=[&](auto&& self,int v)->void {
            visited[v]=1;
            for(int direction=0;direction<4;++direction) {
                int u=grid[v][direction];
                if(visited[u] || s.gained==n*n) continue;
                result.route.push_back(direction);move(s,result.keys,direction);
                self(self,u);
                if(s.gained<n*n) {
                    result.route.push_back(reverse_dir[direction]);
                    move(s,result.keys,reverse_dir[direction]);
                }
            }
        };
        browse(browse,start[0]);
        return result;
    }
    Solution optimize(int mode,bool different,Clock::time_point stop) const {
        Solution result{layouts(mode,different),{}};
        std::vector<int> keys;
        for(int i=0;i<k;++i)
            if(std::none_of(keys.begin(),keys.end(),[&](int j){return result.keys[i]==result.keys[j];}))
                keys.push_back(i);
        State cur=initial();
        while(cur.gained<n*n && int(result.route.size())<2*n*n) {
            if(Clock::now()>=stop) return {};
            auto picks=find_actions(cur,result.keys,keys,mode);
            if(picks.empty()) picks.push_back(fallback_dir(cur,mode));
            for(int j:picks) {
                result.route.push_back(j);
                move(cur,result.keys,j);
                if(cur.gained==n*n || int(result.route.size())==2*n*n) break;
            }
        }
        if(cur.gained<n*n) return {};
        return result;
    }
};

int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    const auto deadline=Clock::now()+std::chrono::milliseconds(1650);
    Problem model;
    if(!model.read()) return 1;
    Solution best=model.dfs();
    auto consider=[&](bool different) {
        for(int mode=0;mode<4 && Clock::now()<deadline;++mode) {
            auto next=model.optimize(mode,different,deadline);
            if(!next.route.empty() && next.route.size()<best.route.size()) best=std::move(next);
        }
    };
#ifndef AHC052_INDIVIDUAL_KEYS
    consider(false);
#endif
#ifndef AHC052_COMMON_KEYS_ONLY
    consider(true);
#endif
    for(const auto& button:best.keys) {
        for(int i=0;i<Robots;++i) std::cout<<Symbol[button[i]]<<(i+1==Robots?'\n':' ');
    }
    for(int key:best.route) std::cout<<key<<'\n';
    std::cerr<<"Model = "<<3*model.n*model.n-best.route.size()<<'\n';
}
