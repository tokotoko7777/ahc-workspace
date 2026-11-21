#define AHC006_NO_MAIN
#define AHC006_GEOMETRIC_SELECTION
#include "../../../../examples/search/ahc006_sa.cpp"
int main(int argc,char** argv){
 ofstream ids(argv[1],ios::binary);
 for(int k=0;k<16;++k){
  mt19937 r(17236+k); DeliveryProblem p;
  int grid=k%4==0?8:801;
  for(auto& x:p.orders)x={int(r()%grid),int(r()%grid),int(r()%grid),int(r()%grid)};
  auto t=chrono::steady_clock::now(); p.prepare_nearby();
  double ms=chrono::duration<double,milli>(chrono::steady_clock::now()-t).count();
  for(auto& nn:p.nearby) ids.write(reinterpret_cast<char*>(nn.data()), sizeof(nn));
  cout<<k<<' '<<ms<<'\n';
 }
}
