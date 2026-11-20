#define AHC_EARLY_CORRIDOR
#define main ahc001_submission_main
#include "../problems/ahc001/main.cpp"
#undef main

int main() {
    int total_pairs=0, total_biaxis=0;
    for (int trial=0; trial<25; ++trial) {
        auto initial=std::make_unique<Solver>(trial);
        auto changed=std::make_unique<Solver>(trial);
        for (Solver* s : {initial.get(),changed.get()}) {
            s->n=16;
            for(int i=0;i<s->n;++i) {
                int a=1300+(i%4)*1100+((i*trial*7)%355);
                int b=1200+(i/4)*1100+((i*trial*11)%361);
                s->x[i]=(trial&1)?b:a; s->y[i]=(trial&1)?a:b;
                s->desired[i]=1000000+(i*1413227)%7500000;
            }
            s->reset_points(); s->remember(); s->refresh_targets(); s->prepare_empty_boxes();
        }
        initial->ordered_initialization();
        changed->early_corridor_initialization();
        assert(changed->early_pairs<=6);
        total_pairs+=changed->early_pairs; total_biaxis+=changed->early_biaxis;
        assert(changed->value>=initial->value-1e-9);
        assert(changed->best_value>=initial->best_value-1e-9);
        for(const Rect* r : {changed->state,changed->best}) for(int i=0;i<changed->n;++i) {
            assert(changed->valid(i,r[i]));
            for(int j=i+1;j<changed->n;++j) assert(!overlap(r[i],r[j]));
        }
        if(!changed->early_admitted) {
            assert(!memcmp(initial->state,changed->state,changed->n*sizeof(Rect)));
            assert(!memcmp(initial->best,changed->best,changed->n*sizeof(Rect)));
            assert(initial->rng.s==changed->rng.s && initial->accepted==changed->accepted);
        } else {
            for(auto [i,j,axis]:changed->early_axes)
                assert(changed->state[i].q[axis+2]<=changed->state[j].q[axis]
                       || changed->state[j].q[axis+2]<=changed->state[i].q[axis]);
        }
    }
    assert(total_pairs>0 && total_biaxis>0);
}
