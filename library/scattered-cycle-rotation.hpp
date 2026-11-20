#pragma once
#include <cassert>
#include <span>
#include <type_traits>
#include <utility>

// Rotate values in distinct indexed cells by one step, in place (direction +/-1).
// report(destination, value) sees each new value once, after it is written.
// It may update an external inverse index, but must not change the board or cycle.
template<class Board,class Report>
void rotate_scattered_cycle(Board& board,std::span<const int> cycle,int direction,Report report) {
    assert(cycle.size()>=2 && (direction==1 || direction==-1));
    using Value=std::remove_cvref_t<decltype(board[cycle.front()])>;
    const int n=cycle.size();
    const int stop=direction==1 ? 0 : n-1;
    const int start=direction==1 ? n-1 : 0;
    Value carry=std::move(board[cycle[start]]);
    for(int i=start;i!=stop;i-=direction) {
        board[cycle[i]]=std::move(board[cycle[i-direction]]);
        report(cycle[i],board[cycle[i]]);
    }
    board[cycle[stop]]=std::move(carry);
    report(cycle[stop],board[cycle[stop]]);
}
