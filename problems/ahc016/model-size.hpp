#pragma once
// Codebooks are node-order invariant, so synthetic trials need only edge flips.
// The fixed RNG here is unrelated to the interactive judge's random seed.
using SizeClock=chrono::steady_clock;
struct NoiseRng {
  uint64_t state;
  uint32_t next() {
    state^=state<<13;
    state^=state>>7;
    state^=state<<17;
    return state>>25;
  }
};

inline optional<double> error_estimate(const DegreeCode& book,double p,int replicates,
                                       uint64_t seed,SizeClock::time_point stop) {
  NoiseRng rng{seed};
  const uint64_t bound=lround(p*4294967296.0);
  int total=0,errors=0;
  vector<double> hist(book.n);
  for(int rep=0;rep<replicates;++rep){
    for(int label=0;label<int(book.code.size());++label){
      if(SizeClock::now()>=stop)return nullopt;
      const auto& graph=book.code[label];
      array<int,100> degrees{};
      for(int u=0,k=0;u<book.n;++u)for(int v=u+1;v<book.n;++v,++k){
        const bool flipped=(rng.next()<bound);
        if((graph[k]=='1')!=flipped)++degrees[u],++degrees[v];
      }
      fill(hist.begin(),hist.end(),0.0);
      for(int u=0;u<book.n;++u)++hist[degrees[u]];
      errors+=book.decode_hist(hist)!=label;
      ++total;
    }
  }
  return double(errors)/total;
}

inline unique_ptr<Codebook> model_size(int m,double p,SizeClock::time_point stop) {
  const int old=choose_n(m,p);
  vector<int> dimensions{old,max(5,old*3/5),max(5,old*4/5),min(100,old*6/5),
                         32,52,72,88,100};
  dimensions.erase(unique(dimensions.begin(),dimensions.end()),dimensions.end());
  struct Row {
    unique_ptr<DegreeCode> graph;
    double merit;
    int n;
  };
  const auto score=[](int n,double e){return 100*log1p(-0.1*e)-log(n);};
  vector<Row> coarse;
  for(int n:dimensions){
    if(ranges::any_of(coarse,[&](const auto& r){return r.n==n;}))continue;
    if(!coarse.empty() && SizeClock::now()>=stop)break;
    auto model=make_unique<DegreeCode>(m,p,n);
    auto e=error_estimate(*model,p,12,0xf3018ee331ULL,stop);
    if(!e){
      if(coarse.empty())return model;
      break;
    }
    coarse.push_back({std::move(model),score(n,*e),n});
  }
  assert(!coarse.empty());
  ranges::sort(coarse,[](const Row& a,const Row& b){
    return pair{a.merit,-a.n}>pair{b.merit,-b.n};
  });
  vector<int> finalists;
  for(int i=0;i<int(coarse.size());++i)
    if(i<2 || coarse[i].n==old)finalists.push_back(i);
  int best=0;
  double highest=-1e100;
  bool complete=true;
  for(int i:finalists){
    auto e=error_estimate(*coarse[i].graph,p,32,0x252890311dULL,stop);
    if(!e){complete=false;break;}
    double final_score=score(coarse[i].n,*e);
    if(final_score>highest)highest=final_score,best=i;
  }
  if(!complete)best=0;
  return std::move(coarse[best].graph);
}
