#include <cassert>
#define AHC_FORCED_TWIST
#define main ahc001_solver_entry
#include "../problems/ahc001/main.cpp"
#undef main

int main() {
  for(int mirror:{0,1}){
    Solver solver(0);
    solver.n=3;
    solver.x[0]=40;solver.y[0]=40;
    solver.x[1]=60;solver.y[1]=60;
    solver.x[2]=150;solver.y[2]=150;
    solver.desired[0]=solver.desired[1]=1200;
    solver.desired[2]=1;
    Rect original[]={{ {20,30,50,70} },{ {50,30,80,70} },{ {150,150,151,151} }};
    for(int i=0;i<3;++i){
      if(mirror){std::swap(original[i].q[0],original[i].q[1]);std::swap(original[i].q[2],original[i].q[3]);}
      solver.state[i]={{solver.x[i],solver.y[i],solver.x[i]+1,solver.y[i]+1}};
    }
    const int found=solver.forced_twist(0,{0,1},original,
               std::chrono::steady_clock::now()+std::chrono::seconds(1));
    assert(found==1);
    const int old=mirror,new_axis=1-mirror;
    const auto a=solver.state[0],b=solver.state[1];
    assert(std::max(a.q[old],b.q[old])<std::min(a.q[old+2],b.q[old+2]));
    assert(a.q[new_axis+2]<=b.q[new_axis]);
    for(int i=0;i<3;++i){
      assert(solver.valid(i,solver.state[i]));
      for(int j=0;j<i;++j)assert(!overlap(solver.state[i],solver.state[j]));
    }
  }
}
