#define AHC006_NO_MAIN
#define AHC006_REUSE_PAIR_ROWS
#include "../../../../examples/search/ahc006_sa.cpp"

int main() {
  mt19937 gen(653141);
  DeliveryProblem problem;
  for(auto& o: problem.orders) {
    o={int(gen()%801),int(gen()%801),int(gen()%801),int(gen()%801)};
  }
  problem.prepare_distances();
  struct Spec { DeliveryProblem::Route r; int a,b,p,d; };
  vector<Spec> v;
  for(int i=0;i<2048;++i) {
    Spec q{};
    q.r.front()=q.r.back()=DEPOT_EVENT;
    vector<int> ids(ORDER_COUNT); iota(ids.begin(),ids.end(),0);
    shuffle(ids.begin(),ids.end(),gen);
    for(int j=0;j<CHOSEN_COUNT;++j){q.r[j*2+1]=2*ids[j];q.r[j*2+2]=2*ids[j]+1;}
    shuffle(q.r.begin()+1,q.r.end()-1,gen);
    for(int a=1;a+1<ROUTE_SIZE;++a) for(int b=a+1;b+1<ROUTE_SIZE;++b)
      if((q.r[a]^1)==q.r[b] && q.r[a]%2)swap(q.r[a],q.r[b]);
    int n=ids[0]; int a=find(q.r.begin(),q.r.end(),n*2)-q.r.begin();
    int b=find(q.r.begin(),q.r.end(),n*2+1)-q.r.begin();
    q.a=a;q.b=b;q.p=ids[CHOSEN_COUNT]*2;q.d=q.p+1; assert(a<b&&problem.is_valid(q.r));
    v.push_back(q);
  }
  auto old=[&](const Spec& q){return best_ordered_pair_insertion(
     DeliveryProblem::WithoutPair{q.r,q.a,q.b}, q.p,q.d,[&](int a,int b){return problem.distance(a,b);});};
  auto fast=[&](const Spec& q){return problem.fast_pair_rows(
     DeliveryProblem::WithoutPair{q.r,q.a,q.b},q.p,q.d);};
  for(const auto& q:v){auto x=old(q),y=fast(q); assert(x.delta==y.delta&&x.first_gap==y.first_gap&&x.second_gap==y.second_gap);}
  auto bench=[&](auto fn){uint64_t checksum=0;auto t=chrono::steady_clock::now();
    for(int i=0;i<150;++i) for(auto& q:v) {auto z=fn(q);checksum+=10000*z.delta+100*z.first_gap+z.second_gap;}
    return pair{checksum,chrono::duration<double,milli>(chrono::steady_clock::now()-t).count()};};
  for(int i=0;i<6;++i){
    pair<uint64_t,double> a,b;if(i%2){b=bench(fast);a=bench(old);}else{a=bench(old);b=bench(fast);}
    assert(a.first==b.first); cout<<a.first<<" old_ms "<<a.second<<" fast_ms "<<b.second<<'\n';
  }
}
