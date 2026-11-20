#pragma once
// Replaying a subsequence removes marks and edges, never introduces blockers.
// Missing prerequisite corners are the only reason to cascade a removal.
inline State replay_without(const State& initial,const vector<Move>& full,int skipped) {
  State s=initial;
  for(int i=0;i<int(full.size());++i) {
    if(i==skipped) continue;
    const auto& op=full[i];
    auto [p,a,b,c]=op.corners();
    if(!s.marks.marked(a.x,a.y) || !s.marks.marked(b.x,b.y) ||
       !s.marks.marked(c.x,c.y)) continue;
    assert(s.legal(op));
    s.apply(op);
  }
  return s;
}

struct CascadeComparison {
  Result suffix,cascade;
  int completed=0, deleted=0, preserved_after=0;
};

inline CascadeComparison compare_cascade(const State& initial,const Result& frozen,
                                          Clock::time_point stop) {
  CascadeComparison out{frozen,frozen};
  if(frozen.moves.size()<4) return out;
  vector<double> weights(initial.n*initial.n);
  for(int x=0;x<initial.n;++x)for(int y=0;y<initial.n;++y)
    weights[x*initial.n+y]=sqrt(initial.weight({x,y})+0.035*initial.n*initial.n);
  const int n=frozen.moves.size();
  Rng seed;
  for(int trial=0;trial<32 && Clock::now()<stop;++trial) {
    int removed=seed.next()%(n*3/4);
    auto cascade=replay_without(initial,frozen.moves,removed);
    int kept=cascade.answer.size(), dropped=n-kept;
    assert(dropped>=1 && kept>=removed);
    State suffix=initial;
    for(int i=0;i<kept;++i) suffix.apply(frozen.moves[i]);
    vector<double> length(2*initial.n+1);
    const double exponent=0.8+1.4*seed.unit(), noise=0.25+0.31*seed.unit();
    for(int i=1;i<int(length.size());++i)length[i]=pow(i,-exponent);
    const uint64_t upper=seed.next(), lower=seed.next();
    const Rng common{(upper<<32)|lower|1u};
    auto grow=[&](State& s,Rng r){
      while(Clock::now()<stop){
        auto next=choose(s,length,weights,noise,r);
        if(!next)return true;
        s.apply(*next);
      }
      return false;
    };
    bool a,b;
    if(trial%2==0){a=grow(suffix,common);b=grow(cascade,common);}
    else{b=grow(cascade,common);a=grow(suffix,common);}
    if(!a||!b) break;
    out.deleted+=dropped;
    out.preserved_after+=kept-removed;
    ++out.completed;
    if(suffix.value>out.suffix.value) out.suffix={std::move(suffix.answer),suffix.value};
    if(cascade.value>out.cascade.value)out.cascade={std::move(cascade.answer),cascade.value};
  }
  return out;
}
