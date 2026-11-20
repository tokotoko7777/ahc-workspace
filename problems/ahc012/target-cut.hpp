#pragma once
// Include after Input, Cuts and State. Threshold c means a*x+b*y <= c.
struct TargetCut {
  int a,b,c;
  array<int,4> endpoints() const {
    int x=0,y=0;
    bool found=false;
    for(int u=-4;u<=4&&!found;++u)for(int v=-4;v<=4&&!found;++v)
      if(a*u+b*v==1) x=u,y=v,found=true;
    assert(found);
    constexpr int B=10000000;
    // Two consecutive normal coordinates with endpoints far outside the cake
    // in the tangent direction, so no integral cake point lies on the line.
    return {c*x-B*b,c*y+B*a,(c+1)*x+B*b,(c+1)*y-B*a};
  }
};

struct TargetComparison {
  int axis_value, all_value, normals=0;
  optional<TargetCut> axis, all;
};

inline TargetComparison target_cut(const Input& in,const Solution& frozen,
                                    Clock::time_point stop) {
  TargetComparison answer{frozen.value,frozen.value,0,{},{}};
  int used=0;
  for(const auto& f:frozen.cuts) used+=f.size();
  if(used>=in.k || frozen.value==in.target) return answer;
  State s(in,frozen.cuts);
  vector<pair<int,int>> events(in.n);
  auto try_normal=[&](int a,int b,bool axis){
    if(Clock::now()>=stop)return;
    for(int i=0;i<in.n;++i)
      events[i]={a*in.pos[i][0]+b*in.pos[i][1],s.key[i]};
    sort(events.begin(),events.end());
    auto candidate=best_capped_partition_split(events,s.count,in.demand);
    assert(candidate.initial_value==frozen.value);
    if(candidate.threshold){
      TargetCut cut{a,b,*candidate.threshold};
      if(axis && candidate.value>answer.axis_value)
        answer.axis_value=candidate.value,answer.axis=cut;
      if(candidate.value>answer.all_value)
        answer.all_value=candidate.value,answer.all=cut;
    }
    ++answer.normals;
  };
  try_normal(1,0,true);
  try_normal(0,1,true);
  for(int a=1;a<=4;++a)for(int b=-4;b<=4;++b)
    if(b!=0 && gcd(a,b)==1)
      try_normal(a,b,false);
  return answer;
}
