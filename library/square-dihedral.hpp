#pragma once
#include <cassert>
#include <utility>

// Transform square coordinates: bit 2 transposes first, bit 0 reflects
// the row, bit 1 reflects the column. Transform id lies in [0, 8).
// For the top-left anchor of a transformed k×k patch in an n×n board,
// pass side=n-k+1, not n.
inline std::pair<int,int> square_dihedral(int row,int col,int side,int id) {
  assert(side>0 && 0<=id && id<8);
  assert(0<=row && row<side && 0<=col && col<side);
  if(id&4)std::swap(row,col);
  if(id&1)row=side-1-row;
  if(id&2)col=side-1-col;
  return {row,col};
}

inline std::pair<int,int> inverse_square_dihedral(int row,int col,int side,int id) {
  // Flip axes must be interchanged when an inverse includes transposition.
  assert(0<=id && id<8);
  if(id&4)id=4|((id&1)<<1)|((id&2)>>1);
  return square_dihedral(row,col,side,id);
}
