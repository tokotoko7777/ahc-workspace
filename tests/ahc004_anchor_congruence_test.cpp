#include "../problems/ahc004/anchor-congruence.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <set>

struct Line {
  int dir, lane, rotation;
  std::string text;
};
int main(int argc,char** argv) {
  const int width=argc>=2?std::clamp(std::stoi(argv[1]),1,2000):128;
  const bool noisy=argc>=3;
  std::array<uint64_t,20> tiny{};
  tiny[0]=3; tiny[1]=1; tiny[2]=2;
  assert(ahc004::match_columns(tiny)==2);
  std::mt19937 rng(609912);
  for (int n : {30,34,36}) {
    int recovered=0,recovered_short=0,rank_sum=0,fourth16=0,fourth1=0;
    double time_sum=0,fourth_ms=0;
    int some=0, realized_right=0, realized_cells=0;
    int64_t exact_sum=0, cap_sum=0;
    for (int rep=0;rep<8;++rep) {
      std::vector<Line> truth;
      std::string board[20];
      for (auto& row:board) for(int p=0;p<20;++p) row+=char('A'+(rng()&7));
      for(int dir=0;dir<2;++dir) for(int lane=0;lane<20;++lane) {
        const int r=rng()%20;
        Line l{dir,lane,r,{}};
        for(int p=0;p<20;++p) l.text+=dir==0?board[lane][(r+p)%20]:board[(r+p)%20][lane];
        truth.push_back(l);
      }
      std::shuffle(truth.begin(),truth.end(),rng);
      if (noisy) for (int id : {0,6}) {
        truth[id].dir = -1;
        for (char& c : truth[id].text) c=char('A'+(rng()&7));
      }
      std::vector<ahc004::Cycle> bank;
      for(int i=0;i<n;++i) bank.push_back({truth[i].text,{},0});
      auto recall=[&](const ahc004::AnchorSearch& out) {
        int rank=0;
        for(const auto& a:out.layouts) {
          ++rank;
          const auto& start=truth[a.id[0]];
          bool yes=start.dir>=0;
          for(int p=1;p<3;++p) {
            const auto& line=truth[a.id[p]];
            yes&=line.dir==start.dir && (line.lane-start.lane+20)%20==a.lane[p] &&
                (start.rotation-line.rotation+20)%20==a.phase[p];
          }
          if(yes) return rank;
        }
        return 0;
      };
      if(rep==0) {
        auto stopped=ahc004::three_parallel_anchors(bank,31);
        assert(stopped.examined==31&&stopped.capped);
        assert(ahc004::three_parallel_anchors(bank,1368000,[] {return true;}).examined==0);
      }
      auto before=std::chrono::steady_clock::now();
      auto out=ahc004::three_parallel_anchors(bank,1368000,[] {return false;},width);
      time_sum+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-before).count();
      assert(out.examined==1368000 && !out.capped);
      exact_sum+=out.exact_checked;
      int rank=recall(out);
      recovered+=rank>0; rank_sum+=rank;
      for(const auto& a:out.layouts) for(int x=0;x<20;++x) for(int i=0;i<n;++i) {
        bool matches=false;
        if(i!=a.id[0]&&i!=a.id[1]&&i!=a.id[2]) for(int p=0;p<20;++p) {
          bool ok=true;
          for(int j=0;j<3;++j) ok&=bank[i].letters[(p+a.lane[j])%20] ==
              bank[a.id[j]].letters[(x+a.phase[j])%20];
          matches|=ok;
        }
        assert(matches == bool((a.domains[x]>>i)&1));
      }
      before=std::chrono::steady_clock::now();
      auto four=ahc004::fourth_row_extensions(bank,out.layouts,width*15000);
      fourth_ms+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-before).count();
      if(!four.ranked.empty()) {
        const auto& f=four.ranked.front();
        const auto& a=out.layouts[f.parent];
        auto r=ahc004::realize_anchor(bank,a,f);
        assert(r.board.size()==20 && r.rows>=4 && r.columns>=f.matched);
        std::set<int> certified;
        const auto check=[&](const std::string& line) {
          bool exists=false;
          for(int id=0;id<n;++id) if((bank[id].letters+bank[id].letters).find(line)<20) {
            exists=true;
            certified.insert(id);
          }
          return exists;
        };
        int complete=0;
        for(int lane=0;lane<20;++lane) {
          const auto& row=r.board[lane];
          std::string col;
          for(const auto& row2:r.board) col+=row2[lane];
          complete += check(row)+check(col);
        }
        assert(complete>=r.rows+r.columns&&int(certified.size())>=r.rows+r.columns);
        const auto& top=truth[a.id[0]];
        if (top.dir>=0) {
          for(int i=0;i<20;++i) for(int j=0;j<20;++j) if(r.board[i][j]!='.') {
            const int y=(top.lane+i)%20, x=(top.rotation+j)%20;
            realized_right+=r.board[i][j]==(top.dir==0?board[y][x]:board[x][y]);
            ++realized_cells;
          }
        }
        ++some;
      }
      for (const auto& f : four.ranked) {
        const auto& a = out.layouts[f.parent];
        std::array<uint64_t,20> masks{};
        for (int c=0;c<20;++c) for (int id=0;id<n;++id) {
          if (id==f.id || !((a.domains[c]>>id)&1)) continue;
          for (int p=0;p<20;++p) {
            bool matches=bank[id].letters[(p+f.lane)%20]==bank[f.id].letters[(c+f.phase)%20];
            for (int j=0;j<3;++j) matches &= bank[id].letters[(p+a.lane[j])%20] ==
                bank[a.id[j]].letters[(c+a.phase[j])%20];
            if (matches) masks[c]|=uint64_t(1)<<id;
          }
        }
        assert(ahc004::match_columns(masks)==f.matched);
        assert(std::count_if(masks.begin(),masks.end(),[](auto m) {return m!=0;})==f.eligible);
      }
      bool exact4=false;
      for(int i=0;i<std::min<int>(16,four.ranked.size());++i) {
        const auto& f=four.ranked[i];
        const auto& a=out.layouts[f.parent];
        const auto& start=truth[a.id[0]];
        const auto& v=truth[f.id];
        ahc004::AnchorSearch singleton;
        singleton.layouts.push_back(a);
        if(recall(singleton)&&v.dir==start.dir&&(v.lane-start.lane+20)%20==f.lane&&
           (start.rotation-v.rotation+20)%20==f.phase) {
          exact4=true;
          fourth1+=i==0;
          break;
        }
      }
      fourth16+=exact4;
      auto finish=std::chrono::steady_clock::now()+std::chrono::milliseconds(18);
      auto short_out=ahc004::three_parallel_anchors(bank,1368000,[&] {
        return std::chrono::steady_clock::now()>=finish;
      });
      recovered_short+=recall(short_out)>0;
      cap_sum+=short_out.examined;
    }
    std::cout<<n<<" full_recall="<<recovered<<"/8 rank_sum="<<rank_sum
             <<" fourth_full_rank16="<<fourth16<<"/8 rank1="<<fourth1<<"/8 fourth_ms="<<fourth_ms/8<<" mean_ms="<<time_sum/8<<" mean_exact="<<exact_sum/8
             <<" realized="<<some<<"/8 actual_cells="<<realized_right<<"/"<<realized_cells<<" 18ms_recall="<<recovered_short<<"/8 mean_cap_visits="<<cap_sum/8<<'\n';
  }
}
