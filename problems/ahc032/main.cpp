// The user's own baseline model, unchanged including attribution.
#define main ahc032_inherited_entry
#ifndef AHC032_BEAM_WIDTH
#define AHC032_BEAM_WIDTH 3500
#endif
#include "../../examples/search/ahc032_action_beam.cpp"
#undef main
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
}
#ifndef AHC032_TEST
int main(){
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  const auto started=Clock::now();
  ModStampProblem p;p.read_input();
  auto initial=p.make_initial_state();
  ActionBeamRunner<ModStampProblem> beam(p,initial,p.initial_score(),AHC032_BEAM_WIDTH);
  int depth=0;
  while(depth<PLACEMENT_COUNT&&Clock::now()-started<chrono::milliseconds(1460)){
    if(!beam.step())break;
    ++depth;
  }
  auto state=beam.best();
  // Always produce a complete legal schedule if the inherited beam was cut.
  while(state.position<PLACEMENT_COUNT){
    const auto& v=p.generate_actions(state);
    auto pick=v.front();
    auto value=p.evaluate_action(state,pick);
    for(auto q:v)if(auto c=p.evaluate_action(state,q);c>value){value=c;pick=q;}
    p.apply_action(state,pick);
  }
  p.validate(state);
  auto actions=decode(p,state);
  Board board=state.board;
  const long long initial_score=accumulate(board.begin(),board.end(),0LL);
  const int accepted=AHC032_REPAIR?repair(p,board,actions,started+chrono::milliseconds(1780)):0;
  const long long score=accumulate(board.begin(),board.end(),0LL);
  cout<<actions.size()<<'\n';
  for(auto a:actions)cout<<a.stamp<<' '<<a.row<<' '<<a.column<<'\n';
  cerr<<"initial="<<initial_score<<" final="<<score<<" depth="<<depth<<" accepted="<<accepted<<'\n';
}
#endif
