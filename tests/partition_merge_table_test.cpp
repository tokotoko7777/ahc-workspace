#include <cassert>
#include "../library/partition-merge-table.hpp"
template<int K> void run(int count) {
    const PartitionMergeTable<K> tb;
    assert(int(tb.labels.size())==count);
    assert(tb.singletons==count-1);
    assert(tb.connected[0]);
    for(int s=0;s<count;++s)
        for(int b=1;b<K;++b) for(int a=0;a<b;++a) {
            const auto j=tb.pair_id(a,b);
            int t=tb.next[s][j];
            assert(t<=s);
            const auto old=tb.labels[s],next=tb.labels[t];
            for(int x=0;x<K;++x)for(int y=0;y<K;++y){
                bool was_connected=old[x]==old[y];
                bool both_merged=(old[x]==old[a]||old[x]==old[b]) &&
                                 (old[y]==old[a]||old[y]==old[b]);
                assert((next[x]==next[y])==(was_connected||both_merged));
            }
        }
}
int main(){
    run<1>(1);run<2>(2);run<3>(5);run<4>(15);run<5>(52);run<6>(203);run<7>(877);
}
