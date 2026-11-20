// This unit is appended to a copy of the bundled production translation unit
// with only its CLI entry point renamed. Pocket stays internal to AHC001.
int main() {
    mt19937 rng(912451);
    int improved=0;
    for(int trial=0;trial<600;++trial){
        int n=2+rng()%5;
        Portfolio p(n);
        for(int i=0;i<n;++i){
            p[i].push_back({{{i*4,0,i*4+1,1}},0});
            for(int j=0;j<3;++j){
                int x=int(rng()%21)-1,y=int(rng()%4)-1,w=1+rng()%9,h=1+rng()%4;
                p[i].push_back({{{x,y,x+w,y+h}},(int(rng()%31)-10)*0.0001});
            }
        }
        Pocket model(p);
        auto [best,assignment]=model.solve(true);
        assert(model.checks<model.cap);
        auto [local,lassign]=Pocket(p).solve(false);
        double brute=0;
        for(int code=0;code<(1<<(2*n));++code){
            double total=0;bool ok=true;
            for(int i=0;i<n;++i){
                int a=code>>(2*i)&3;
                total+=p[i][a].gain;
                for(int j=0;j<i;++j)if(overlap(p[i][a].box,p[j][code>>(2*j)&3].box))ok=false;
            }
            if(ok)brute=max(brute,total);
        }
        auto verify=[&](const auto& a,double expected){
            double actual=0;
            for(int i=0;i<n;++i){
                actual+=p[i][a[i]].gain;
                for(int j=0;j<i;++j)assert(!overlap(p[i][a[i]].box,p[j][a[j]].box));
            }
            assert(abs(actual-expected)<1e-12);
        };
        verify(assignment,best);verify(lassign,local);
        assert(abs(brute-best)<1e-12 && local<=best+1e-12);
        improved+=best>local+1e-12;
    }
    assert(improved>0);
    cout<<"600 exhaustive pocket checks passed; strict exact advantage on "<<improved<<" cases\n";
}
