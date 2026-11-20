#define AHC040_TEST
#include "../problems/ahc040/main.cpp"

void validate(vector<Dim> truth,const vector<Move>& plan) {
  vector<Pose> p;
  for(auto m:plan){
    auto r=slide(truth[m.rect],p,m);
    bool certified=m.dir=='U'?r.y0==0:r.x0==0;
    const int width=r.x1-r.x0,height=r.y1-r.y0;
    auto [w,h]=truth[m.rect];
    if(m.rot)swap(w,h);
    assert(width==w&&height==h);
    for(auto q:p){
      bool ox=overlap(r.x0,r.x1,q.x0,q.x1),oy=overlap(r.y0,r.y1,q.y0,q.y1);
      assert(!ox||!oy);
      certified|=m.dir=='U'?(ox&&r.y0==q.y1):(oy&&r.x0==q.x1);
    }
    assert(certified);
    p.push_back(r);
  }
}
int main(){
  mt19937 rng(400040);
  for(int t=0;t<140;++t){
    vector<Dim> d(30+rng()%71);
    for(auto& [a,b]:d){a=10000+rng()%90001;b=10000+rng()%90001;}
    auto noisy=dimensions(d,1000+rng()%9001,t);
    validate(d,shelf(noisy,t));
    validate(d,mixed(noisy,t));
  }
  vector<Pose> p{{0,0,80000,100000}};
  auto r=slide({50000,40000},p,Move{1,0,'U',0});
  assert(r.x0==80000&&r.y0==0); // Edge touching does not block.
}
