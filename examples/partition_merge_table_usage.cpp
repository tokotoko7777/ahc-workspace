#include <iostream>
#include "../library/partition-merge-table.hpp"
int main(){
    const PartitionMergeTable<4> table;
    auto s=table.singletons;
    s=table.next[s][table.pair_id(0,1)];
    s=table.next[s][table.pair_id(2,3)];
    s=table.next[s][table.pair_id(1,3)];
    std::cout<<int(table.connected[s])<<'\n';
}
