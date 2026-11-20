#include <fstream>
#include <iostream>
#include "fixed_walk_relocation.hpp"

int main(int argc, char** argv) {
  if (argc != 2) return 2;
  std::ifstream instance(argv[1]);
  int n,m,k,t;
  if (!(instance>>n>>m>>k>>t) || n<=0 || m<0 || k<=0 || k>n || t<0) return 2;
  std::vector<std::vector<int>> graph(n);
  for (int a,b; m--; ) {
    if (!(instance>>a>>b) || a<0 || a>=n || b<0 || b>=n) return 2;
    graph[a].push_back(b); graph[b].push_back(a);
  }
  std::vector<int> actions;
  for (int op; std::cin>>op;) actions.push_back(op);
  if (!std::cin.eof()) return 2;
  try {
    auto result=ahc060::relocate(graph,k,t,actions);
    for (int op:result.actions) std::cout<<op<<'\n';
    std::cerr<<result.before<<" -> "<<result.after<<'\n';
  } catch (const std::exception& e) {
    std::cerr<<e.what()<<'\n'; return 2;
  }
}
