#define AHC_PAIRED_ROLLOUT
#define AHC_MATCHED_REPLACEMENT
#define main ahc007_solution_main
#include "../problems/ahc007/main.cpp"
#undef main
#include <cassert>

int main() {
    Dsu chosen(N);
    chosen.unite(1,2);
    std::vector<Edge> tree{{0,1,5,36},{2,3,5,14},{3,4,6,29},{2,5,3,99}};
    assert(final_release(chosen,tree,0,4)==36);
    assert(final_release(chosen,tree,4,5)==99);
    assert(final_release(chosen,tree,1,2)==-1);
    assert(final_release(chosen,tree,1,6)==-1);
}
