#define AHC010_NO_MAIN
#define AHC010_PATCH_TEST
#include "../problems/ahc010/main.cpp"

static std::array<int,2> directed(const Board& b,const ContractedPatch& p,
                                  const ContractedPatch::Rot& r){
  auto tile=b.orientation;
  for(int j=0;j<4;++j){
    int t=b.input[p.cells[j]];
    for(int k=0;k<r[j];++k)t=rotate_tile[t];
    tile[p.cells[j]]=t;
  }
  std::array<int,4*N> visited{};
  std::vector<int> loops;
  for(int v=0;v<4*N;++v){
    if(visited[v])continue;
    int u=v, length=0;
    while(u>=0&&!visited[u]){
      visited[u]=v+1;
      ++length;
      int d=to[tile[u/4]][u%4];
      if(d<0){u=-1;break;}
      int w=b.adj[u/4][d];
      u=w>=0?4*w+(d^2):-1;
    }
    if(u==v) loops.push_back(length);
  }
  std::sort(loops.rbegin(),loops.rend());
  assert(loops.size()%2==0);
  for(size_t j=0;j<loops.size();j+=2)assert(loops[j]==loops[j+1]);
  loops.resize(std::max(size_t(4),loops.size()));
  return {loops[0],loops[2]};
}

int main(){
  std::mt19937 rng(101042);
  for(int k=0;k<30;++k){
    Board b;
    for(int& x:b.input)x=(k%3==0)?4+rng()%2:rng()%8; // long dense chains as well
    b.initialize();
    for(int i=0;i<N;++i)b.store(i,rng()%b.period[i]);
    for(int i=0;i<N;++i)b.build_links(i);
    for(int q=0;q<14;++q){
      ContractedPatch p(b,(rng()%(H-1))*W+rng()%(W-1));
      for(int x=0;x<20;++x){
        ContractedPatch::Rot r{};
        for(int j=0;j<4;++j)r[j]=rng()%b.period[p.cells[j]];
        assert(p.longest(r)==directed(b,p,r));
      }
      for(int i:p.cells)b.store(i,rng()%b.period[i]);
      for(int i:p.cells)b.refresh(i);
    }
  }
}
