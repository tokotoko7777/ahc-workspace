#define AHC005_TEST
#include "../examples/search/ahc005_patrol_sa.cpp"
#include "../problems/ahc005/path-aware.hpp"

int main(){
    Solver map;map.read_input();map.build_segments();
    PatrolProblem p(map);
    std::mt19937 rng(98123);
    using Leg=DirectedPathCoverage::Leg;
    for(int trial=0;trial<100;++trial){
        DirectedPathCoverage test(map);
        auto order=p.initial_state().order;
        std::vector<int> nodes;
        for(auto a:order)nodes.push_back(a.point);
        int n=int(nodes.size());
        auto visit=[&](int i){return nodes[i%n];};
        auto cycle=[&](int i){return Leg{visit(i),visit(i+1)};};
        for(int i=0;i<n;++i)test.change(cycle(i),1);
        assert(test.cells.all_covered()==map.route_covers_every_road(nodes));
        for(int step=0;step<30;++step){
            for(int i=0;i<n;++i)test.change(cycle(i),-1);
            assert(test.cells.uncovered()==int(map.roads.size()));
            if(step%2==0&&n>2) {
                nodes.erase(nodes.begin()+1+rng()%(n-1));
                n=int(nodes.size());
            } else if(n>2){
                std::swap(nodes[1+rng()%(n-1)],nodes[1+rng()%(n-1)]);
            }
            for(int i=0;i<n;++i)test.change(cycle(i),1);
            assert(test.cells.all_covered()==map.route_covers_every_road(nodes));
        }
    }
}
