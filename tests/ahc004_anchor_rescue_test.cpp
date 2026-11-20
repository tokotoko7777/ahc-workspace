#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3")
#endif
#include "../problems/ahc004/anchor-congruence.hpp"
#include <random>
#include <cassert>
#include <iostream>
#include <chrono>
int main() {
  std::mt19937 rng(1173254);
  std::string grid[20];
  for(auto& row:grid) for(int i=0;i<20;++i) row+=char('A'+(rng()&7));
  std::vector<ahc004::Cycle> bank;
  // Completely false top quintet. Subsequent five are true rows; the
  // remaining inventory contains 16 true columns and ten more true rows.
  for(int i=0;i<5;++i) {
    std::string f;
    for(int j=0;j<20;++j) f+=char('A'+(rng()&7));
    bank.push_back({f,{},1000-i});
  }
  for(int lane=0;lane<15;++lane)bank.push_back({grid[lane],{},990-lane});
  for(int lane=0;lane<16;++lane){
    std::string col;
    for(int y=0;y<20;++y) col+=grid[y][lane];
    bank.push_back({col,{},950-lane});
  }
  assert(ahc004::three_parallel_anchors(bank,20,[] {return false;},50,{5,5,6,7,8}).examined==0);
  auto old=ahc004::three_parallel_anchors(bank,1368000,[] {return false;},128);
  auto rescue=ahc004::three_parallel_anchors(bank,1368000,[] {return false;},128,{5,6,7,8,9});
  assert(old.examined==1368000 && rescue.examined==1368000);
  auto check=[&](const auto& a){
    return a.id[0]>=5 && a.id[2]<20 && a.lane[1]==a.id[1]-a.id[0] &&
      a.lane[2]==a.id[2]-a.id[0] && a.phase[1]==0 && a.phase[2]==0;
  };
  assert(std::none_of(old.layouts.begin(),old.layouts.end(),check));
  assert(std::any_of(rescue.layouts.begin(),rescue.layouts.end(),check));
  auto now=std::chrono::steady_clock::now();
  auto regular=ahc004::fourth_row_extensions(bank,rescue.layouts,2000000);
  auto mid=std::chrono::steady_clock::now();
  auto packed=ahc004::fourth_row_extensions(bank,rescue.layouts,2000000,[] {return false;},true);
  auto end=std::chrono::steady_clock::now();
  assert(regular.examined==packed.examined && regular.ranked.size()==packed.ranked.size());
  for(size_t j=0;j<regular.ranked.size();++j){
    auto a=regular.ranked[j], b=packed.ranked[j];
    assert((std::tuple(a.parent,a.id,a.lane,a.phase,a.matched,a.eligible) ==
      std::tuple(b.parent,b.id,b.lane,b.phase,b.matched,b.eligible)));
  }
  assert(!packed.ranked.empty());
  std::vector<std::string> reads;
  for(int lane=0;lane<20;++lane) {
    std::string col;
    for(int y=0;y<20;++y)col+=grid[y][lane];
    for(auto row:{grid[lane],col}) for(int start=0;start<20;++start)
      reads.push_back((row+row).substr(start,9+(start%3)));
  }
  const auto score=[&](const std::vector<std::string>& b){
    std::vector<std::string> lines;
    for(int lane=0;lane<20;++lane){
      std::string col;
      for(int r=0;r<20;++r)col+=b[r][lane];
      lines.push_back(b[lane]+b[lane]); lines.push_back(col+col);
    }
    return std::count_if(reads.begin(),reads.end(),[&](const auto& s){
      return std::any_of(lines.begin(),lines.end(),[&](const auto& l){return l.find(s)<20;});
    });
  };
  int previous=-1,best=-1,realized=0,unique=0;
  std::vector<std::string> keys;
  for(const auto& f:packed.ranked) {
    auto b=ahc004::realize_anchor(bank,rescue.layouts[f.parent],f);
    if(b.board.empty())continue;
    std::vector<std::string> transformed(20,std::string(20,'.'));
    for(int r=0;r<20;++r)for(int c=0;c<20;++c)transformed[r][c]=b.board[(c+1)%20][(r+3)%20];
    auto key=ahc004::geometry_shape_key(b.board);
    assert(key==ahc004::geometry_shape_key(transformed));
    if(++realized==1)previous=score(b.board);
    if(std::find(keys.begin(),keys.end(),key)==keys.end()){
      ++unique;
      keys.push_back(key);
      best=std::max(best,int(score(b.board)));
    }
    if(realized==32) break;
  }
  assert(realized>0 && previous>=0 && best>=previous);
  std::cout<<"synthetic observed reads first/best "<<previous<<'/'<<best
           <<" from "<<realized<<" realized / "<<unique<<" unique; fourth regular/packed ms "
           <<std::chrono::duration<double,std::milli>(mid-now).count()<<"/"
           <<std::chrono::duration<double,std::milli>(end-mid).count()<<std::endl;
}
