#include "../library/square-dihedral.hpp"
#include <cassert>
#include <set>
#include <utility>

int main(){
  for(int n=1;n<=17;++n)for(int id=0;id<8;++id){
    std::set<std::pair<int,int>> seen;
    for(int r=0;r<n;++r)for(int c=0;c<n;++c){
      auto [a,b]=square_dihedral(r,c,n,id);
      assert(seen.emplace(a,b).second);
      assert((inverse_square_dihedral(a,b,n,id)==std::pair{r,c}));
    }
    for(int w=1;w<=n;++w){
      int anchors=n-w+1;
      for(int r=0;r<anchors;++r)for(int c=0;c<anchors;++c){
        auto [ar,ac]=square_dihedral(r,c,anchors,id);
        for(int dy=0;dy<w;++dy)for(int dx=0;dx<w;++dx){
          auto [y,x]=square_dihedral(dy,dx,w,id);
          assert((square_dihedral(r+dy,c+dx,n,id)==std::pair{ar+y,ac+x}));
        }
      }
    }
  }
}
