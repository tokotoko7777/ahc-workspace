// Optional experiment. Keep the original example and its attribution intact.
#define main ahc026_original_entry
#include "../../examples/search/ahc026_deterministic_rollout.cpp"
#undef main

#ifndef AHC026_SNAPSHOT_SPLIT
#define AHC026_SNAPSHOT_SPLIT 1
#endif
int main(int argc,char** argv){
  using Plan=BoxRolloutProblem::Plan;
  using State=BoxRolloutProblem::State;
  using Clock=chrono::steady_clock;
  const auto deadline=Clock::now()+chrono::milliseconds(1650);
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  BoxRolloutProblem p;
  State state=p.read_initial_state();
  const auto initial=state.stacks;
  DeterministicRolloutRunner<BoxRolloutProblem> runner(p,false);
  runner.reserve(p.n+1);
  vector<State> snapshots;
  while(state.next_target<=p.n){
    int target=state.next_target,from=state.location[target];
    auto& s=state.stacks[from];
    if(int(s.end()-find(s.begin(),s.end(),target))>=3)snapshots.push_back(state);
    int action=runner.choose_action(state);
    p.apply_action(state,action);
  }
  Plan best=state.answer;
  const int incumbent_energy=best.energy;
  int evaluations=0, edited=0, processed=0;
  const bool allow_split=argc>1 ? string_view(argv[1])=="split" : AHC026_SNAPSHOT_SPLIT;
  // Equal count: nine distinct edits and four exact completions per snapshot.
  for(const auto& snap:snapshots){
    if(processed>=24 || Clock::now()>=deadline) break;
    int target=snap.next_target, from=snap.location[target];
    const auto& pile=snap.stacks[from];
    int first=int(find(pile.begin(),pile.end(),target)-pile.begin())+1;
    vector<int> destinations;
    for(int d=0;d<p.m;++d)if(d!=from)destinations.push_back(d);
    stable_sort(destinations.begin(),destinations.end(),[&](int a,int b){
      return p.minimum_box(snap.stacks[a])>p.minimum_box(snap.stacks[b]);
    });
    vector<tuple<int,int,int,int>> choices;
    if(allow_split){
      int len=int(pile.size());
      auto risk=[&](int l,int r,int d){
        int minimum=p.minimum_box(snap.stacks[d]);
        int affected=max(0,*min_element(pile.begin()+l,pile.begin()+r)-minimum);
        int inversions=0;
        for(int i=l;i<r;++i)for(int j=i+1;j<r;++j)if(pile[i]<pile[j])++inversions;
        return (affected>0 ? 20+min(50,affected)/3 : 0)+(r-l)*(affected>0)+3*inversions;
      };
      for(int cut=first+1;cut<len;++cut)for(int a:destinations)for(int b:destinations){
        if(a==b)continue;
        choices.emplace_back(risk(cut,len,a)+risk(first,cut,b),cut,a,b);
      }
      sort(choices.begin(),choices.end());
    }else{
      for(int d:destinations)choices.emplace_back(0,-1,-1,d);
    }
    choices.resize(min(choices.size(),destinations.size()));
    if(choices.size()!=destinations.size())throw logic_error("not enough distinct edits");
    ++processed;
    for(auto [risk,cut,a,b]:choices){
      State trial=snap;
      if(cut!=-1)p.move_suffix(trial.stacks,trial.location,from,cut,a,trial.answer);
      p.move_suffix(trial.stacks,trial.location,from,first,b,trial.answer);
      p.apply_action(trial,0); // The target is already on top.
      for(int range:{0,2,6,15}){
        ++evaluations;
        auto tail=p.make_plan(trial,range);
        if(trial.answer.energy+tail.energy>=best.energy ||
           trial.answer.operations.size()+tail.operations.size()>5000)continue;
        best=trial.answer;
        best.energy+=tail.energy;
        best.operations.insert(best.operations.end(),tail.operations.begin(),tail.operations.end());
        edited=target;
      }
    }
  }
  validate_or_throw(initial,p.n,p.m,best);
  for(auto [box,dst]:best.operations)cout<<box<<' '<<dst<<'\n';
  cerr<<"incumbent="<<incumbent_energy<<" energy="<<best.energy<<" snapshots="<<processed
      <<" evaluations="<<evaluations<<" edit="<<edited<<'\n';
}
