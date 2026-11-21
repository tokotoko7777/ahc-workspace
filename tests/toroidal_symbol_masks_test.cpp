#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3")
#endif
#include "../library/toroidal-symbol-masks.hpp"
#include <cassert>
#include <random>
int main(){
    std::mt19937 rng(17223755);
    for(int rep=0;rep<42;++rep){
        const int n=1+rep%20;
        std::vector<std::string> b(n,std::string(n,'.'));
        for(auto& row:b)for(char& c:row)c="AAAABBCDEF..G.H"[rng()%15];
        ahc004::ToroidalSymbolMasks m(b);
        for(int step=0;step<55;++step){
            int a=rng()%n,c=rng()%n;
            char fresh="ABCDEFGH.."[rng()%10];
            m.set(a,c,b[a][c],fresh);
            b[a][c]=fresh;
            int holes=0;
            for(auto& row:b)for(char v:row)holes+=v=='.';
            assert(m.dots()==holes);
            int len=1+rng()%n;
            int source=rng()%(2*n),p=rng()%n;
            std::string w(len,'A');
            for(int i=0;i<len;++i){
                char d=source<n?b[source][(p+i)%n]:b[(p+i)%n][source-n];
                w[i]=d=='.'?char('A'+rng()%8):d;
                if(step%4==0)w[i]=char('A'+rng()%8);
            }
            bool present=false;
            for(int l=0;l<2*n;++l){
                unsigned exact=0,loose=0;
                int filled=0;
                for(int q=0;q<n;++q)filled+=(l<n?b[l][q]:b[q][l-n])!='.';
                assert(m.filled_in_line(l)==filled);
                for(int s=0;s<n;++s){
                    bool ok=true,permits=true;
                    int empties=0;
                    for(int j=0;j<len;++j){
                        char d=l<n?b[l][(s+j)%n]:b[(s+j)%n][l-n];
                        ok &= d==w[j];
                        permits &= d=='.'||d==w[j];
                        empties+=d=='.';
                    }
                    assert(m.dots_in_window(l,s,len)==empties);
                    exact|=unsigned(ok)<<s;
                    loose|=unsigned(permits)<<s;
                }
                present|=exact!=0;
                assert(m.starts<false>(l,w.data(),len)==exact);
                assert(m.starts<true>(l,w.data(),len)==loose);
            }
            assert(m.present(w)==present);
        }
    }
}
