#define AHC018_NO_MAIN
#include "../problems/ahc018/main.cpp"

int main(){
  Solver s;
  s.n=8; s.c=16; s.k=2; s.w=1;
  s.partial.assign(64,0);
  s.hardness.assign(64,0);
  s.hole.assign(64,0);
  s.wet.assign(64,0);
  std::istringstream replies("0\n1\n2\n");
  std::ostringstream commands;
  auto* in=std::cin.rdbuf(replies.rdbuf());
  auto* out=std::cout.rdbuf(commands.rdbuf());
  s.partial_probe(12,200);
  assert(!s.hole[12] && s.partial[12]==200 && s.samples.size()==1);
  assert(s.remaining_model(12,50)==100);
  assert(s.remaining_model(12,349)==149);
  assert(s.remaining_model(13,50)==50);
  s.drill(12,50);
  assert(s.hole[12] && s.hardness[12]>200 && !s.ended);
  s.partial_probe(13,620);
  assert(s.hole[13]&&s.ended);
  const auto earlier=commands.str();
  s.drill(14,500);
  assert(earlier==commands.str());
  std::cin.rdbuf(in);
  std::cout.rdbuf(out);
}
