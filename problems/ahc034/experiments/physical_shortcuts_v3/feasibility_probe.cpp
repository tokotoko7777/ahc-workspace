// Diagnose why a coverage-preserving proposed walk cannot be assigned cargo.
#define AHC034_SHORTCUTS 2
#define main ahc034_submission_main
#include "../../main.cpp"
#undef main

static int prefix_floor(const Problem& p,const Route& r){
    array<int,V> first,last;
    first.fill(-1);last.fill(-1);
    for(int t=0;t<int(r.size());++t){
        int v=r[t];
        if(first[v]<0)first[v]=t;
        last[v]=t;
    }
    int load=0,low=0;
    for(int v=0;v<V;++v)if(p.height[v])assert(first[v]>=0&&last[v]>=0);
    for(int t=0;t<int(r.size());++t){
        int v=r[t],h=p.height[v];
        if((h>0&&first[v]==t)||(h<0&&last[v]==t))load+=h;
        low=min(low,load);
    }
    assert(load==0);
    return low;
}
int main(int argc,char** argv){
    if(argc==1){
        Problem p;
        p.height[1]=-2;p.height[2]=2;
        assert(prefix_floor(p,{0,1,2})==-2);
        assert(prefix_floor(p,{0,1,2,1})==0);
        p.height[5]=1;p.height[6]=-1;
        assert(prefix_floor(p,{0,1,2,3,4,5,6,1})==0);
        return 0;
    }
    if(argc!=3)return 2;
    ifstream input(argv[1]),out(argv[2]);
    auto saved=cin.rdbuf(input.rdbuf());
    Problem p;p.read();cin.rdbuf(saved);
    Route initial{0};
    string line;
    while(getline(out,line)){
        int step=line=="D"?N:line=="U"?-N:line=="L"?-1:line=="R"?1:0;
        if(step)initial.push_back(initial.back()+step);
    }
    assert(prefix_floor(p,initial)==0);
    for(int i=0;i<2;++i){
        int trials=0,edits=0;
        const auto r=p.shorten(initial,i?56:24,i!=0,Clock::now()+chrono::seconds(1),trials,edits);
        cout<<r.size()-1<<' '<<prefix_floor(p,r)<<' ';
    }
    cout<<'\n';
}
