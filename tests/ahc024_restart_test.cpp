#define AHC024_NO_MAIN
#include "../problems/ahc024/main.cpp"

int main(){
  for(int t=0;t<24;++t){
    int n=9+t%8;
    Board board{n,n,vector<int>(n*n)};
    for(int y=0;y<n;++y)for(int x=0;x<n;++x)
      board.v[y*n+x]=1+(3*y/n)*3+3*x/n;
    Topology topo(board,9);
    Board first=board;
    auto stop=Clock::time_point::max();
    straight(first,topo,stop);
    const auto before=first.colored();
    Board same=first,diverse=first;
    auto a=multistart(board,first,topo,same,stop,false);
    auto b=multistart(board,first,topo,diverse,stop,true);
    assert(a.completed==10&&b.completed==10&&a.distinct==1&&b.distinct>=1);
    assert(topo.valid(same)&&topo.valid(diverse));
    assert(same.colored()<=before&&diverse.colored()<=before);
  }
}
