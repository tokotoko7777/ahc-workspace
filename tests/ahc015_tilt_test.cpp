#define AHC015_NO_MAIN
#include "../problems/ahc015/main.cpp"

int main(){
  std::mt19937 rng(150647);
  constexpr int dr[]={-1,1,0,0},dc[]={0,0,-1,1};
  for(int t=0;t<2000;++t){
    Board src{};
    for(auto& i:src)i=rng()%4;
    for(int d=0;d<4;++d){
      Board ref=src;
      bool move;
      do{
        move=false;
        for(int i=0;i<T;++i)if(ref[i]){
          int r=i/N+dr[d],c=i%N+dc[d];
          if(r>=0&&r<N&&c>=0&&c<N&&!ref[r*N+c]){
            std::swap(ref[i],ref[r*N+c]);move=true;
          }
        }
      }while(move);
      assert(tilt(src,d)==ref);
    }
  }
}
