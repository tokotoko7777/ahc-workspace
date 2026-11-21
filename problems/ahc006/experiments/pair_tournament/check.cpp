#define AHC006_NO_MAIN
#define AHC006_REUSE_PAIR_ROWS
#define AHC006_PAIR_TOURNAMENT
#include "../../../../examples/search/ahc006_sa.cpp"
int main() {
  mt19937_64 rng(473); DeliveryProblem p;
  for(auto& x:p.orders)x={int(rng()%801),int(rng()%801),int(rng()%801),int(rng()%801)};
  p.prepare_distances(); auto s=p.make_initial_state(); int count=0, replaced=0;
  for(int i=0;i<120000;++i) {
    auto m=p.propose_move(s,rng,(i%1000)*.001);
    if(!m)continue; auto copy=s; int before=s.cost; p.apply_move(copy,*m);
    assert(copy.cost==before+m->delta&&copy.cost==p.route_cost(copy.route)&&p.is_valid(copy.route));
    ++count;
    if(m->kind==DeliveryProblem::Replace)++replaced;
    if(i%7==0||m->delta<=0) s=copy;
  }
  cout<<count<<" legal exact moves, "<<replaced<<" tested replacements\n";
}
