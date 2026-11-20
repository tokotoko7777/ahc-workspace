#define AHC_CUT_SHIFT_HEADROOM
#define main main_inherited
#include "../../main.cpp"
#undef main

struct Opening {int goal, blocker, axis, side, to; double priority;};

static bool cross(Rect a,Rect b,int axis) {
    return a.q[axis]<b.q[axis+2]&&b.q[axis]<a.q[axis+2];
}
static void validate(const Solver& p) {
    for(int i=0;i<p.n;++i) {
        if(!p.valid(i,p.state[i]))throw std::runtime_error("anchor or wall");
        for(int j=0;j<i;++j)
            if(overlap(p.state[i],p.state[j]))throw std::runtime_error("overlap");
    }
}
static double score(const Solver& p) {
    double s=0.;
    for(int i=0;i<p.n;++i)s+=quality(p.state[i].area(),p.desired[i]);
    return s;
}
static std::vector<Opening> openings(const Solver& p){
    std::vector<Opening> a;
    for(int i=0;i<p.n;++i) {
        Rect gi=p.state[i];
        double deficit=1.-double(gi.area())/p.desired[i];
        if(deficit<=.01)continue;
        for(int j=0;j<p.n;++j)if(i!=j)for(int axis=0;axis<2;++axis) {
            int t=axis^1;
            Rect bj=p.state[j];
            int gap=std::max(bj.q[axis]-gi.q[axis+2],gi.q[axis]-bj.q[axis+2]);
            if(gap<0||!cross(gi,bj,t))continue;
            int anchor=t?p.y[j]:p.x[j];
            for(int side:{t,t+2}) {
                int target=side<2?gi.q[t+2]:gi.q[t];
                if(side<2 && (target<=bj.q[t]||target>anchor))continue;
                if(side>=2 && (target>=bj.q[t+2]||target<=anchor))continue;
                Rect shrunk=bj;shrunk.q[side]=target;
                if(!p.valid(j,shrunk)||cross(gi,shrunk,t))throw std::logic_error("bad shrink");
                double loss=quality(bj.area(),p.desired[j])-quality(shrunk.area(),p.desired[j]);
                int edge=t?gi.w():gi.h();
                int desired_space=std::max(1,int(std::ceil(double(p.desired[i]-gi.area())/edge)));
                double room=std::min(1.0,double(bj.q[axis+2]-bj.q[axis])/desired_space);
                double priority=deficit*deficit*(0.5+room) - std::max(0.,loss)*0.22 - gap*0.00002;
                a.push_back({i,j,axis,side,target,priority});
            }
        }
    }
    std::sort(a.begin(),a.end(),[](Opening a,Opening b){
        if(a.priority!=b.priority)return a.priority>b.priority;
        return std::tie(a.goal,a.blocker,a.axis,a.side)<std::tie(b.goal,b.blocker,b.axis,b.side);
    });
    if(a.size()>12)a.resize(12);
    return a;
}
static bool improve_one_side(Solver&p,int i,int d) {
    Rect r=p.state[i];int axis=d&1,t=axis^1;
    int bound=d<2?0:10000;
    for(int j=0;j<p.n;++j)if(j!=i&&cross(r,p.state[j],t)) {
        Rect b=p.state[j];
        if(d<2&&b.q[axis+2]<=r.q[axis])bound=std::max(bound,b.q[axis+2]);
        if(d>=2&&r.q[axis+2]<=b.q[axis])bound=std::min(bound,b.q[axis]);
    }
    int across=r.q[t+2]-r.q[t];
    double desired=double(p.desired[i])/across;
    int old=r.q[d],lo=std::min(old,bound),hi=std::max(old,bound),best_coord=old;
    double val=quality(r.area(),p.desired[i]);
    for(int width:{int(std::floor(desired)),int(std::ceil(desired))}){
        int z=d<2?r.q[d+2]-width:r.q[d-2]+width;
        r.q[d]=std::clamp(z,lo,hi);
        double q=quality(r.area(),p.desired[i]);
        if(q>val+1e-14) val=q,best_coord=r.q[d];
    }
    p.state[i].q[d]=best_coord;
    return old!=best_coord;
}
static int burst(Solver& p,int axis){
    int n=0;
    for(int d:{1024,512,256,128,64,16,4,1}) {
        n+=p.cut_shift(axis,d);
        n+=p.cut_shift(axis,-d);
        n+=p.cut_shift(axis^1,d);
        n+=p.cut_shift(axis^1,-d);
    }
    return n;
}
int main(int argc,char** argv) {
    if(argc!=6)return 2;
    Solver p(0);
    std::ifstream in(argv[1]),init(argv[2]);
    in>>p.n;
    if(p.n<=0||p.n>200)return 3;
    for(int i=0;i<p.n;++i){
        in>>p.x[i]>>p.y[i]>>p.desired[i];
        for(int d=0;d<4;++d)init>>p.state[i].q[d];
    }
    if(!in||!init)return 4;
    validate(p);
    const auto start=std::chrono::steady_clock::now();
    const double original=score(p);
    // Both contenders begin with identical deterministic pure cut polishing.
    burst(p,0);burst(p,1);validate(p);
    const double common=score(p);
    Rect common_state[205],best_pure[205],best_top[205];
    std::copy_n(p.state,p.n,common_state);
    std::copy_n(p.state,p.n,best_pure);std::copy_n(p.state,p.n,best_top);
    double pure=common,top=common;int candidates=0,switches=0,top_better=0,regrown=0;
    const auto o=openings(p);
    for(auto a:o) {
        // A whole control+topology pair is kept. Limit number of pairs and wall clock.
        if(std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()>.11)break;
        ++candidates;
        std::copy_n(common_state,p.n,p.state);
        burst(p,a.axis);
        for(int pass=0;pass<2;++pass)for(int d=0;d<4;++d) improve_one_side(p,a.blocker,(a.side+d)%4);
        burst(p,a.axis^1);validate(p);
        const double s0=score(p);
        if(s0>pure+1e-13)pure=s0,std::copy_n(p.state,p.n,best_pure);
        std::copy_n(common_state,p.n,p.state);
        p.state[a.blocker].q[a.side]=a.to;validate(p);
        burst(p,a.axis);
        for(int pass=0;pass<2;++pass)for(int d=0;d<4;++d) improve_one_side(p,a.blocker,(a.side+d)%4);
        burst(p,a.axis^1);validate(p);
        const bool switched=cross(p.state[a.goal],p.state[a.blocker],a.axis);
        const double s1=score(p);
        switches+=switched;
        regrown+=p.state[a.blocker].area()>common_state[a.blocker].area()*0.99;
        top_better+=s1>s0+1e-13;
        // Compare with original common-state score after both cuts and regrowth.
        // Non-improving candidates are rejected by resetting the snapshot.
        if(s1>top+1e-13)top=s1,std::copy_n(p.state,p.n,best_top);
    }
    auto dump=[&](const char* path,Rect* q) {
        std::copy_n(q,p.n,p.state);validate(p);
        std::ofstream out(path);
        for(int i=0;i<p.n;++i){
            for(int d=0;d<4;++d)out<<q[i].q[d]<<(d==3?'\n':' ');
        }
        if(!out)throw std::runtime_error("output failed");
    };
    auto rounded=[&](double s){return std::llround((s*1e9)/p.n);};
    dump(argv[3],common_state);
    dump(argv[4],best_pure);
    dump(argv[5],best_top);
    std::cout<<rounded(original)<<' '<<rounded(common)<<' '<<rounded(pure)<<' '
             <<rounded(top)<<' '<<candidates<<' '<<switches<<' '<<top_better<<' '<<regrown<<'\n';
}
