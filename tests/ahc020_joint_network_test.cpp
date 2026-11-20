#define AHC020_NO_MAIN
#include "../problems/ahc020/main.cpp"

int main(){
  std::mt19937 rng(204275);
  for(int t=0;t<80;++t){
    Instance in{};
    in.n=4+rng()%18;
    in.mst_parent.resize(in.n);
    in.mst_edge.resize(in.n,-1);
    for(int i=0;i<in.n;++i){
      in.mst_order.push_back(i);
      if(i){
        in.mst_parent[i]=int(rng()%i);
        in.mst_edge[i]=in.edges.size();
        in.edges.push_back({i,in.mst_parent[i],1+int(rng()%100)});
      }
    }
    for(int i=2;i<in.n;++i)for(int j=0;j<i;++j)
      if(rng()%5==0&&j!=in.mst_parent[i])in.edges.push_back({i,j,1+int(rng()%150)});
    in.m=in.edges.size();
    in.shortest_paths();
    JointCost check{in};
    std::vector<int> powers(in.n),fallback;
    std::vector<unsigned char> scratch;
    for(int r=0;r<20;++r){
      for(auto& v:powers)v=rng()%4?int(rng()%5000+1):0;
      auto c=in.mst(powers,scratch,&fallback);
      auto ret=steiner(in,powers,fallback,c,Clock::time_point::max());
      Dsu dsu(in.n);
      Int actual=0;
      for(int e=0;e<in.m;++e)if(ret[e]){
        const auto& v=in.edges[e];
        actual+=v.w;dsu.unite(v.a,v.b);
      }
      for(int k=0;k<in.n;++k)if(powers[k])assert(dsu.same(0,k));
      assert(actual<=c && check(powers)==actual);
      int previous=check.misses;
      for(auto& v:powers)if(v)v=1+rng()%5000;
      powers[0]=!powers[0];
      assert(check(powers)==actual && check.misses==previous);
    }
  }
}
