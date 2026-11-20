#define AHC034_SHORTCUTS 3
#define main ahc034_submission_main
#include "../problems/ahc034/main.cpp"
#undef main

static bool reference(const Problem& p,const Route& r){
    std::array<int,V> first,last;
    first.fill(-1);last.fill(-1);
    for(int i=0;i<int(r.size());++i){
        if(first[r[i]]<0)first[r[i]]=i;
        last[r[i]]=i;
    }
    vector<int> changes(r.size());
    for(int i=0;i<V;++i)if(p.height[i]){
        assert(first[i]>=0);
        changes[p.height[i]>0?first[i]:last[i]]+=p.height[i];
    }
    int h=0;
    for(int v:changes){h+=v;if(h<0)return false;}
    return h==0;
}
int main(){
    mt19937 rng(340371);
    int saw_yes=0,saw_no=0;
    for(int i=0;i<4000;++i){
        Problem p;
        for(int v=0;v<5;++v){
            int mass=int(rng()%17)-8;
            p.height[v]=mass;p.height[5]-=mass;
        }
        Route old;
        for(int t=0;t<24;++t)old.push_back(rng()%6);
        int l=rng()%16,r=l+1+rng()%(23-l);
        Route ins;
        for(int t=rng()%9;t>0;--t)ins.push_back(rng()%6);
        Route candidate(old.begin(),old.begin()+l+1);
        candidate.insert(candidate.end(),ins.begin(),ins.end());
        candidate.insert(candidate.end(),old.begin()+r+1,old.end());
        array<int,V> counts{},delta{},final{};
        for(int v:old)++counts[v];
        for(int t=l+1;t<=r;++t)--delta[old[t]];
        for(int v:ins)++delta[v];
        for(int v:candidate)++final[v];
        bool cover=true;
        for(int v=0;v<6;++v){
            assert(final[v]==counts[v]+delta[v]);
            if(p.height[v] && !final[v])cover=false;
        }
        if(cover){
            bool actual=p.chronological_splice(old,l,r,ins,counts,delta);
            assert(actual==reference(p,candidate));
            (actual?saw_yes:saw_no)++;
        }
    }
    assert(saw_yes>500 && saw_no>500);
    cout<<saw_yes<<" feasible, "<<saw_no<<" infeasible exact prefix differentials\n";
}
