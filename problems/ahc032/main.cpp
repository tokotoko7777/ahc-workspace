// The user's own baseline model, unchanged including attribution.
#define main ahc032_inherited_entry
#ifndef AHC032_BEAM_WIDTH
#define AHC032_BEAM_WIDTH 3500
#endif
#include "../../examples/search/ahc032_action_beam.cpp"
#undef main
#include "../../library/square-dihedral.hpp"
#ifndef AHC032_PORTFOLIO
#define AHC032_PORTFOLIO 0
#endif
#ifndef AHC032_REPAIR
#define AHC032_REPAIR 0
#endif
namespace {
using Clock=chrono::steady_clock;
using Board=array<uint32_t,BOARD_SIZE*BOARD_SIZE>;
struct Press{int stamp,row,column;};

uint32_t shift_value(uint32_t before,uint32_t by,int sign){
  if(sign>0){const auto x=before+by;return x>=MODULO?x-MODULO:x;}
  return before>=by?before-by:before+MODULO-by;
}
long long stamp(Board& board,const ModStampProblem& p,Press move,int sign){
  long long delta=0;
  for(int r=0;r<3;++r)for(int c=0;c<3;++c){
    auto& x=board[(move.row+r)*BOARD_SIZE+move.column+c];
    const uint32_t after=shift_value(x,p.stamps[move.stamp][r*3+c],sign);
    delta+=static_cast<long long>(after)-x;
    x=after;
  }
  return delta;
}
vector<Press> decode(const ModStampProblem& p,const ModStampProblem::State& s){
  vector<Press> out;
  for(int step=0;step<PLACEMENT_COUNT;++step){
    auto& combo=p.combinations[s.choices[step]];
    for(int k=0;k<combo.count;++k)
      out.push_back({combo.stamp_ids[k],p.placements[step].row,p.placements[step].column});
  }
  return out;
}
int repair(const ModStampProblem& p,Board& board,vector<Press>& actions,
           Clock::time_point deadline){
  if(actions.size()<2)return 0;
  mt19937 rng(320032);
  int accepts=0;
  for(int k=0;k<3400;++k){
    if((k&15)==0&&Clock::now()>=deadline)break;
    const int i=rng()%actions.size();
    array<int,OPERATION_LIMIT> nearby;
    int count=0;
    Press a=actions[i];
    for(int id=0;id<int(actions.size());++id)if(id!=i&&
      abs(actions[id].row-a.row)<=2&&abs(actions[id].column-a.column)<=2)nearby[count++]=id;
    if(count==0)continue;
    int j=nearby[rng()%count];
    Press b=actions[j],trialA=a,trialB=b;
    if(k%3!=0){
      Press& q=k%3==1?trialA:trialB;
      int axis=rng()%2;
      int& z=axis?q.row:q.column;
      z=clamp(z+(rng()%2?1:-1),0,PLACEMENTS_PER_AXIS-1);
    }
    long long base=stamp(board,p,a,-1);
    base+=stamp(board,p,b,-1);
    long long improve=0;
    for(int x=0;x<STAMP_COUNT;++x){
      trialA.stamp=x;
      const long long da=base+stamp(board,p,trialA,1);
      for(int y=0;y<STAMP_COUNT;++y){
        trialB.stamp=y;
        const long long db=stamp(board,p,trialB,1);
        if(da+db>improve){
          improve=da+db;
          actions[i]=trialA;actions[j]=trialB;
        }
        stamp(board,p,trialB,-1);
      }
      stamp(board,p,trialA,-1);
    }
    stamp(board,p,actions[i],1);
    stamp(board,p,actions[j],1);
    accepts+=improve>0;
  }
  return accepts;
}
ModStampProblem rotated(const ModStampProblem& original,int transform){
  ModStampProblem p;
  for(int y=0;y<9;++y)for(int x=0;x<9;++x){
    auto [a,b]=square_dihedral(y,x,9,transform);
    p.initial_board[a*9+b]=original.initial_board[y*9+x];
  }
  for(int k=0;k<STAMP_COUNT;++k)for(int y=0;y<3;++y)for(int x=0;x<3;++x){
    auto [a,b]=square_dihedral(y,x,3,transform);
    p.stamps[k][a*3+b]=original.stamps[k][y*3+x];
  }
  p.prepare();
  return p;
}
struct SearchResult{
  Board board;
  vector<Press> actions;
  long long score=0,work=0;
  int depth=0;
};
SearchResult solve_beam(
    ModStampProblem& p,int width,Clock::time_point deadline){
  auto initial=p.make_initial_state();
  ActionBeamRunner<ModStampProblem> beam(p,initial,p.initial_score(),width);
  int depth=0;
  long long work=0;
  while(depth<PLACEMENT_COUNT&&Clock::now()<deadline){
    for(auto& s:beam.states())work+=p.generate_actions(s).size();
    if(!beam.step())break;
    ++depth;
  }
  auto state=beam.best();
  // Always emit a complete feasible answer even if the search was interrupted.
  while(state.position<PLACEMENT_COUNT){
    const auto& v=p.generate_actions(state);
    auto pick=v.front();
    auto value=p.evaluate_action(state,pick);
    for(auto q:v)if(auto c=p.evaluate_action(state,q);c>value){value=c;pick=q;}
    p.apply_action(state,pick);
  }
  p.validate(state);
  return {state.board,decode(p,state),
          accumulate(state.board.begin(),state.board.end(),0LL),work,depth};
}
}
#ifndef AHC032_TEST
int main(){
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  const auto started=Clock::now();
  ModStampProblem p;p.read_input();
  const auto beam_deadline=started+chrono::milliseconds(1460);
  SearchResult result;
  long long work=0;
  int first_depth=0,second_depth=0,orientation=0;
  if(AHC032_PORTFOLIO){
    result=solve_beam(p,AHC032_BEAM_WIDTH/2,beam_deadline);
    work=result.work;first_depth=result.depth;
    if(Clock::now()<beam_deadline){
      auto reverse=rotated(p,3);
      auto second=solve_beam(reverse,AHC032_BEAM_WIDTH-AHC032_BEAM_WIDTH/2,beam_deadline);
      second_depth=second.depth;
      work+=second.work;
      if(second.score>result.score){
        result=std::move(second);
        orientation=3;
        Board reoriented{};
        for(int r=0;r<9;++r)for(int c=0;c<9;++c){
          auto [y,x]=inverse_square_dihedral(r,c,9,orientation);
          reoriented[y*9+x]=result.board[r*9+c];
        }
        result.board=reoriented;
        for(auto& a:result.actions)
          tie(a.row,a.column)=inverse_square_dihedral(a.row,a.column,PLACEMENTS_PER_AXIS,orientation);
      }
    }
  }else{
    result=solve_beam(p,AHC032_BEAM_WIDTH,beam_deadline);
    work=result.work;first_depth=result.depth;
  }
  auto& board=result.board;
  auto& actions=result.actions;
  const int accepted=AHC032_REPAIR?repair(p,board,actions,started+chrono::milliseconds(1780)):0;
  const long long score=accumulate(board.begin(),board.end(),0LL);
  cout<<actions.size()<<'\n';
  for(auto a:actions)cout<<a.stamp<<' '<<a.row<<' '<<a.column<<'\n';
  cerr<<"initial="<<result.score<<" final="<<score<<" depth="<<first_depth<<" accepted="<<accepted
      <<" second="<<second_depth<<" orientation="<<orientation<<" work="<<work<<'\n';
}
#endif
