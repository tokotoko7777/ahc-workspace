#define main v64_submission
#include "../../main.cpp"
#undef main
namespace {
struct Option {Rect box; double gain;};
using Portfolio=vector<vector<Option>>;
struct Pocket {
    Portfolio p;
    uint64_t checks=0, cap=9000;
    vector<vector<vector<uint32_t>>> conflicts;
    explicit Pocket(Portfolio options):p(move(options)) {
        int n=p.size();
        conflicts.resize(n);
        for(int i=0;i<n;++i) {
            conflicts[i].resize(p[i].size(),vector<uint32_t>(n));
            for(int a=0;a<int(p[i].size());++a)
                for(int j=0;j<n;++j) if(i!=j)
                    for(int b=0;b<int(p[j].size());++b)
                        if(overlap(p[i][a].box,p[j][b].box)) conflicts[i][a][j]|=1u<<b;
        }
    }
    pair<double,vector<int>> solve(bool exact){
        const int n=p.size();
        vector<int> best(n), tentative(n);
        double gain=0;
        if(!exact){
            for(;;) {
                double delta=1e-13;
                int qi=-1,qj=-1,qa=-1,qb=-1;
                for(int i=0;i<n && checks<cap;++i)for(int j=i+1;j<n && checks<cap;++j)
                    for(int a=0;a<int(p[i].size()) && checks<cap;++a)
                        for(int b=0;b<int(p[j].size()) && checks<cap;++b){
                            double d=p[i][a].gain+p[j][b].gain-p[i][best[i]].gain-p[j][best[j]].gain;
                            if(d<=delta)continue;
                            ++checks;
                            if(conflicts[i][a][j]>>b&1)continue;
                            bool okay=true;
                            for(int k=0;k<n;++k)if(k!=i && k!=j && ((conflicts[i][a][k]|conflicts[j][b][k])>>best[k]&1)){
                                okay=false;break;
                            }
                            if(okay)delta=d,qi=i,qj=j,qa=a,qb=b;
                        }
                if(qi<0) break;
                best[qi]=qa;best[qj]=qb;gain+=delta;
            }
            return {gain,best};
        }
        vector<vector<int>> order(n);
        vector<uint32_t> available(n);
        for(int i=0;i<n;++i){
            order[i].resize(p[i].size());iota(order[i].begin(),order[i].end(),0);
            stable_sort(order[i].begin(),order[i].end(),[&](int a,int b){return p[i][a].gain>p[i][b].gain;});
            available[i]=(1u<<p[i].size())-1;
        }
        auto search=[&](auto&& self,vector<uint32_t> mask,int open,double accumulated)->void{
            if(!open){if(accumulated>gain+1e-13)gain=accumulated,best=tentative;return;}
            int i=-1,small=INT_MAX; double upper=accumulated;
            for(int m=open;m;m&=m-1){
                int k=countr_zero(unsigned(m)), count=popcount(mask[k]);
                if(!count)return;
                if(count<small)small=count,i=k;
                for(int a:order[k])if(mask[k]>>a&1){upper+=p[k][a].gain;break;}
            }
            if(upper<=gain+1e-13)return;
            int rest=open^(1<<i);
            for(int a:order[i]) if(mask[i]>>a&1){
                if(checks>=cap)return;
                ++checks;
                auto next=mask;
                for(int m=rest;m;m&=m-1){
                    int j=countr_zero(unsigned(m));
                    next[j]&=~conflicts[i][a][j];
                }
                tentative[i]=a;
                self(self,move(next),rest,accumulated+p[i][a].gain);
            }
        };
        search(search,available,(1<<n)-1,0);
        return {gain,best};
    }
};
Rect best_clip(const Solver& s,int id,Rect b,const vector<int>& fixed,int prefer) {
    for(int k:fixed)if(overlap(b,s.state[k])){
        Rect chosen{}; long long priority=-1;
        for(int t=0;t<4;++t){
            int d=(t+prefer)%4;
            Rect v=b;v.q[d]=s.state[k].q[d^2];
            if(s.valid(id,v) && v.area()>priority)priority=v.area(),chosen=v;
        }
        if(priority<0)return s.state[id];
        b=chosen;
    }
    return b;
}
Portfolio candidates(const Solver& s,const vector<int>& ids){
    vector<int> fixed;
    for(int i=0;i<s.n;++i)if(find(ids.begin(),ids.end(),i)==ids.end())fixed.push_back(i);
    Portfolio result;
    for(int id:ids){
        auto old=s.state[id];
        double q0=quality(old.area(),s.desired[id]);
        vector<Option> options{{old,0}};
        auto add=[&](Rect c){
            if(!s.valid(id,c))return;
            for(int k:fixed)if(overlap(c,s.state[k]))return;
            if(any_of(options.begin(),options.end(),[&](auto& v){return equal(v.box.q,v.box.q+4,c.q);}))return;
            options.push_back({c,quality(c.area(),s.desired[id])-q0});
        };
        for(int t=0;t<4;++t){
            Rect shrink=old;
            int side=t&1?s.y[id]:s.x[id];
            shrink.q[t]=(shrink.q[t]+side+(t>=2))/2;
            add(shrink);
        }
        auto nearest=fixed;
        auto distance=[&](int k){const Rect b=s.state[k];return max({0,b.q[0]-s.x[id],s.x[id]+1-b.q[2]})
                                                           +max({0,b.q[1]-s.y[id],s.y[id]+1-b.q[3]});};
        stable_sort(nearest.begin(),nearest.end(),[&](int a,int b){return distance(a)<distance(b);});
        for(int t=0;t<6;++t){
            int pad=t<3?650:1500;
            Rect b=old;
            for(int d=0;d<4;++d)b.q[d]=clamp(b.q[d]+(d<2?-pad:pad),0,10000);
            if(t%3==1){b.q[1]=old.q[1];b.q[3]=old.q[3];}
            if(t%3==2){b.q[0]=old.q[0];b.q[2]=old.q[2];}
            b=best_clip(s,id,b,nearest,t);
            if(!s.valid(id,b))continue;
            int target=min(b.area(),s.desired[id]);
            int lo=(target+b.h()-1)/b.h(),hi=min(target,b.w());
            for(int width:{lo,clamp(old.w(),lo,hi),hi}){
                int height=clamp(int(lround(double(target)/width)),1,b.h());
                auto xpos=pair{max(b.q[0],s.x[id]-width+1),min(s.x[id],b.q[2]-width)};
                auto ypos=pair{max(b.q[1],s.y[id]-height+1),min(s.y[id],b.q[3]-height)};
                // Keep one anchor-near shape and one offset shape in each run.
                int l=clamp(old.q[0],xpos.first,xpos.second),bot=clamp(old.q[1],ypos.first,ypos.second);
                add({{l,bot,l+width,bot+height}});
                if(t%2){
                    l=t%3?xpos.second:xpos.first;bot=t%3?ypos.first:ypos.second;
                    add({{l,bot,l+width,bot+height}});
                }
            }
        }
        if(options.size()>24) {
            stable_sort(options.begin()+1,options.end(),[](const auto& a,const auto& b){return a.gain>b.gain;});
            options.resize(24);
        }
        result.push_back(move(options));
    }
    return result;
}
vector<vector<int>> fields(const Solver& s){
    vector<vector<int>> ids;
    vector<int> centers(s.n);iota(centers.begin(),centers.end(),0);
    auto q=[&](int i){return quality(s.state[i].area(),s.desired[i]);};
    stable_sort(centers.begin(),centers.end(),[&](int a,int b){return q(a)<q(b);});
    for(int i:centers){
        auto group=centers;
        const Rect a=s.state[i];
        auto key=[&](int j){const Rect b=s.state[j];return pair{
            max({0,a.q[0]-b.q[2],b.q[0]-a.q[2]})+max({0,a.q[1]-b.q[3],b.q[1]-a.q[3]}),
            abs(s.x[i]-s.x[j])+abs(s.y[i]-s.y[j])};};
        stable_sort(group.begin(),group.end(),[&](int x,int y){return key(x)<key(y);});
        group.resize(min(6,s.n));sort(group.begin(),group.end());
        if(find(ids.begin(),ids.end(),group)==ids.end())ids.push_back(group);
        if(ids.size()==3)break;
    }
    return ids;
}
}
int main(int argc,char** argv){
    if(argc!=4)return 2;
    ifstream in(argv[1]), incumbent(argv[2]); Solver s(0);
    in>>s.n;
    if(s.n<2 || s.n>205)return 2;
    for(int i=0;i<s.n;++i){
        in>>s.x[i]>>s.y[i]>>s.desired[i];
        for(auto& b:s.state[i].q)incumbent>>b;
        assert(in&&incumbent&&s.valid(i,s.state[i]));
        for(int j=0;j<i;++j)assert(!overlap(s.state[i],s.state[j]));
    }
    std::array<Rect,205> best;
    copy(s.state,s.state+s.n,best.begin());
    double gain=0; int options=0,improves=0,capped=0;
    uint64_t checks=0;
    auto windows=fields(s);
    for(const auto& ids:windows){
        Pocket model(candidates(s,ids));
        for(const auto& v:model.p)options+=v.size();
        auto [score,choice]=model.solve(string_view(argv[3])=="exact");
        checks+=model.checks;capped+=model.checks==model.cap;
        if(score>1e-13)++improves;
        if(score<=gain+1e-13)continue;
        copy(s.state,s.state+s.n,best.begin());
        gain=score;
        for(int i=0;i<int(ids.size());++i)best[ids[i]]=model.p[i][choice[i]].box;
        for(int j=0;j<s.n;++j)if(find(ids.begin(),ids.end(),j)==ids.end())
            assert(equal(best[j].q,best[j].q+4,s.state[j].q));
    }
    for(int i=0;i<s.n;++i){
        assert(s.valid(i,best[i]));
        for(int j=0;j<i;++j)assert(!overlap(best[i],best[j]));
        cout<<best[i].q[0]<<' '<<best[i].q[1]<<' '<<best[i].q[2]<<' '<<best[i].q[3]<<'\n';
    }
    cerr<<"windows="<<windows.size()<<" options="<<options<<" checks="<<checks<<" capped="<<capped
        <<" improves="<<improves<<" gain="<<llround(gain/s.n*1e9)<<'\n';
}
