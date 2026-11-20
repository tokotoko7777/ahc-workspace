#define AHC_FORCED_TWIST
#define main ahc001_normal_entry
#include "../../main.cpp"
#undef main

namespace {
struct Trial {int i,j,old;double priority;};
struct Result {int attempted=0,feasible=0,improved=0,moved=0;double gain=0;};
int coordinate(const Solver& s,int id,int axis){return axis?s.y[id]:s.x[id];}

Rect bbox(Rect a,Rect b,int padding){
    Rect r;
    for(int d=0;d<2;++d){
        r.q[d]=std::max(0,std::min(a.q[d],b.q[d])-padding);
        r.q[d+2]=std::min(10000,std::max(a.q[d+2],b.q[d+2])+padding);
    }
    return r;
}

int intersection_area(Rect a,Rect b){
    return std::max(0,std::min(a.q[2],b.q[2])-std::max(a.q[0],b.q[0]))*
           std::max(0,std::min(a.q[3],b.q[3])-std::max(a.q[1],b.q[1]));
}

std::optional<Rect> clip_neighbors(const Solver& s,int id,Rect shape,Rect a,Rect b){
    std::vector<Rect> forms{shape};
    for(Rect blocker:{a,b}){
        std::vector<Rect> next;
        for(Rect r:forms) {
            if(!overlap(r,blocker)){next.push_back(r);continue;}
            for(int axis=0;axis<2;++axis)for(int side=0;side<2;++side){
                Rect q=r;
                if(side)q.q[axis]=std::max(q.q[axis],blocker.q[axis+2]);
                else q.q[axis+2]=std::min(q.q[axis+2],blocker.q[axis]);
                if(s.valid(id,q))next.push_back(q);
            }
        }
        forms=std::move(next);
    }
    if(forms.empty())return std::nullopt;
    return *std::max_element(forms.begin(),forms.end(),[&](Rect a,Rect b){
        return quality(a.area(),s.desired[id])<quality(b.area(),s.desired[id]);
    });
}

std::vector<Trial> shortlist(const Solver& s){
    std::vector<Trial> list;
    for(int i=0;i<s.n;++i)for(int j=0;j<i;++j){
        auto a=s.state[i],b=s.state[j];
        bool separate[]={a.q[2]<=b.q[0]||b.q[2]<=a.q[0],
                         a.q[3]<=b.q[1]||b.q[3]<=a.q[1]};
        if(separate[0]==separate[1]||s.x[i]==s.x[j]||s.y[i]==s.y[j])continue;
        int old=!separate[0],gap=std::max(a.q[old]-b.q[old+2],b.q[old]-a.q[old+2]);
        if(gap>180)continue;
        const double defect=2-quality(a.area(),s.desired[i])-quality(b.area(),s.desired[j]);
        const double priority=defect*1e5-gap*2
          -.02*(std::abs(s.x[i]-s.x[j])+std::abs(s.y[i]-s.y[j]));
        list.push_back({i,j,old,priority});
    }
    std::stable_sort(list.begin(),list.end(),[](auto a,auto b){return a.priority>b.priority;});
    if(list.size()>12)list.resize(12);
    return list;
}

double total(const Solver& s,const Rect* r){
    double sum=0;
    for(int i=0;i<s.n;++i)sum+=quality(r[i].area(),s.desired[i]);
    return sum;
}

Result repair(Solver& s,bool twist){
    Result result;
    auto choices=shortlist(s);
    std::array<Rect,205> original{},best{},working{};
    std::copy(s.state,s.state+s.n,original.begin());
    best=original;
    const double before=total(s,original.data());
    double best_value=before;
    for(auto [i,j,old,priority]:choices){
        (void)priority;
        const int separation=twist?1-old:old,seam_axis=1-separation;
        const int first=coordinate(s,i,separation)<coordinate(s,j,separation)?i:j;
        const int second=first==i?j:i;
        const int lo=coordinate(s,first,separation)+1,hi=coordinate(s,second,separation);
        const int p=(coordinate(s,i,seam_axis)+coordinate(s,j,seam_axis))/2;
        const int q=(std::max(original[i].q[seam_axis],original[j].q[seam_axis])+
                     std::min(original[i].q[seam_axis+2],original[j].q[seam_axis+2])-1)/2;
        const auto bounds=bbox(original[i],original[j],512);
        std::vector<std::pair<int,int>> hits;
        for(int u=0;u<s.n;++u)if(u!=i&&u!=j){
            const int intersection=intersection_area(bounds,original[u]);
            if(intersection>0)hits.emplace_back(-intersection,u);
        }
        std::ranges::sort(hits);
        if(hits.size()>3)hits.resize(3);
        std::copy(original.begin(),original.begin()+s.n,s.state);
        for(auto [v,u]:hits){(void)v;s.state[u]={{s.x[u],s.y[u],s.x[u]+1,s.y[u]+1}};}
        for(int padding:{128,512})for(int seam:{p,q})for(int cut:{lo,(lo+hi)/2,hi}){
            ++result.attempted;
            Rect a=bbox(original[i],original[j],padding),b=a;
            a.q[separation+2]=cut;b.q[separation]=cut;
            auto ra=s.fit_seam(first,second,a,seam_axis,seam,original[first]);
            auto rb=s.fit_seam(second,first,b,seam_axis,seam,original[second]);
            if(!ra||!rb)continue;
            working=original;
            working[first]=*ra;working[second]=*rb;
            bool legal=true;
            int moved=0;
            for(auto [v,u]:hits){
                (void)v;
                auto candidate=clip_neighbors(s,u,original[u],*ra,*rb);
                if(!candidate){legal=false;break;}
                working[u]=*candidate;
                moved+=candidate->area()!=original[u].area();
            }
            if(!legal)continue;
            ++result.feasible;
            const double value=total(s,working.data());
            if(value>before+1e-10)++result.improved;
            if(value>best_value+1e-10){
                best_value=value;best=working;result.moved=moved;
            }
        }
    }
    for(int i=0;i<s.n;++i){
        assert(s.valid(i,best[i]));
        for(int j=0;j<i;++j)assert(!overlap(best[i],best[j]));
        s.state[i]=best[i];
    }
    result.gain=best_value-before;
    return result;
}
}

int main(int argc,char** argv){
    if(argc!=3)return 2;
    std::ifstream input(argv[1]),incumbent(argv[2]);
    Solver s(0);
    if(!(input>>s.n))return 2;
    for(int i=0;i<s.n;++i){
        input>>s.x[i]>>s.y[i]>>s.desired[i];
        for(auto& c:s.state[i].q)if(!(incumbent>>c))return 2;
        assert(s.valid(i,s.state[i]));
        for(int j=0;j<i;++j)assert(!overlap(s.state[i],s.state[j]));
    }
    const char* mode=std::getenv("AHC001_LOCAL_TWIST");
    const auto result=repair(s,mode&&std::string_view(mode)=="1");
    std::cerr<<"attempts="<<result.attempted<<" feasible="<<result.feasible
             <<" improves="<<result.improved<<" moved="<<result.moved
             <<" gain="<<std::llround(result.gain*1e9/s.n)<<'\n';
    for(int i=0;i<s.n;++i){
        const auto& v=s.state[i].q;
        std::cout<<v[0]<<' '<<v[1]<<' '<<v[2]<<' '<<v[3]<<'\n';
    }
}
