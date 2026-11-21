#pragma GCC optimize("O3")
#include <bits/stdc++.h>
#include "../../../../library/toroidal-symbol-masks.hpp"
using namespace std;
using Board=vector<string>;
using StrSet=vector<pair<string,uint32_t>>;
using namespace std::chrono;
struct Change{int r,c; char from,to;};
template<bool Fast,bool Build,bool Mutate>
pair<double,uint64_t> bench(const vector<Board>& inputs,const vector<StrSet>& words,
                            const vector<vector<Change>>& updates){
    const auto t=steady_clock::now();
    uint64_t found=0;
    for(int d=0;d<(int)inputs.size();++d) {
        ahc004::BasicToroidalSymbolMasks<Fast> masks(inputs[d]);
        for(int j=0;j<16;++j){
            if constexpr(Mutate){
                for(auto u:updates[d]) masks.set(u.r,u.c,j%2?u.to:u.from,j%2?u.from:u.to);
                asm volatile("" : : "g"(&masks) : "memory");
            }
            if constexpr(!Build){
                for(const auto& [word,key]:words[d]) {
                    if constexpr(Fast) found+=masks.present_prefilter(word,key);
                    else found+=masks.present(word);
                }
            }
        }
        asm volatile("" : : "g"(&masks) : "memory");
    }
    return {duration<double,milli>(steady_clock::now()-t).count(),found};
}
template<bool Build,bool Mutate>
void measure(const vector<Board>& b,const vector<StrSet>& w,const vector<vector<Change>>& c){
    vector<double> control,fast;
    for(int i=0;i<5;++i){
        pair<double,uint64_t> a,z;
        if(i%2){ a=bench<false,Build,Mutate>(b,w,c);z=bench<true,Build,Mutate>(b,w,c); }
        else { z=bench<true,Build,Mutate>(b,w,c);a=bench<false,Build,Mutate>(b,w,c); }
        assert(a.second==z.second);
        control.push_back(a.first); fast.push_back(z.first);
    }
    sort(control.begin(),control.end());sort(fast.begin(),fast.end());
    cout<<control[2]<<' '<<fast[2]<<' '<<control[2]/fast[2]<<'\n';
}
int main(){
    mt19937 gen(14746177);
    for(int filled:{25,65,100}) for(int len:{4,7,11}){
        vector<Board> b(20,Board(20,string(20,'.')));
        vector<StrSet> w(20);
        vector<vector<Change>> c(20);
        for(int d=0;d<20;++d){
            for(auto& row:b[d]) for(char& x:row) if(gen()%100<filled)x='A'+gen()%8;
            for(int i=0;i<400;++i){
                string word(len,'A');
                int line=gen()%40,pos=gen()%20;
                for(int j=0;j<len;++j){
                    char x=line<20?b[d][line][(pos+j)%20]:b[d][(pos+j)%20][line-20];
                    word[j]=i%4==0&&x!='.'?x:'A'+gen()%8;
                }
                w[d].push_back({word,ahc004::ToroidalSymbolMasks::signature(word)});
            }
            for(int j=0;j<20;++j)c[d].push_back({j,(j*7)%20,b[d][j][(j*7)%20],char("ABCDEFGH."[gen()%9])});
        }
        cout<<filled<<' '<<len<<" query: ";measure<false,false>(b,w,c);
        cout<<filled<<' '<<len<<" mutation+query: ";measure<false,true>(b,w,c);
        vector<Board> more; vector<StrSet> mw; vector<vector<Change>> mc;
        for(int j=0;j<100;++j){more.insert(more.end(),b.begin(),b.end());mc.insert(mc.end(),c.begin(),c.end());}
        if(len==4){
            cout<<filled<<" construction only: ";measure<true,false>(more,mw,mc);
            cout<<filled<<" construction+mutation: ";measure<true,true>(more,mw,mc);
        }
    }
}
