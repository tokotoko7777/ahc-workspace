#pragma once

inline double separation_penalty(int d){
  return d ? 1.0/(d*d) : 30000.0;
}

inline vector<vector<pair<int,int>>> reverse_locations(
    int cells,const vector<vector<int>>& locations){
  vector<vector<pair<int,int>>> reverse(cells);
  for(int i=0;i<int(locations.size());++i)
    for(int b=0;b<int(locations[i].size());++b)
      reverse[locations[i][b]].emplace_back(i,b);
  return reverse;
}

struct PairChange { double delta=0; bool valid=true; };

inline PairChange check_word_edit(const vector<unsigned>& previous,
                                 const vector<unsigned>& flips,int minimum){
  PairChange result;
  for(int i=0;i<int(previous.size());++i)if(flips[i]){
    for(int j=0;j<int(previous.size());++j){
      if(i==j || (flips[j] && j<i))continue;
      const unsigned before=previous[i]^previous[j];
      int h=std::popcount(before^flips[i]^flips[j]);
      if(h<minimum)return {0,false};
      result.delta+=separation_penalty(h)-separation_penalty(std::popcount(before));
    }
  }
  return result;
}

inline void reduce_energy(int l,const vector<char>& fixed,const vector<vector<int>>& locations,
                          vector<char>& bits,vector<unsigned>& words,Clock::time_point stop){
  const auto reverse=reverse_locations(l*l,locations);
  vector<vector<pair<int,int>>> near(l*l);
  vector<int> points;
  for(int i=0;i<l*l;++i)if(fixed[i]){
    points.push_back(i);
    for(int dir=0;dir<4;++dir){
      const int dy=dir==0?1:dir==1?-1:0;
      const int dx=dir==2?1:dir==3?-1:0;
      auto at=[&](int k){return ((i/l+l+k*dy)%l)*l+(i%l+l+k*dx)%l;};
      const int a=at(1),b=at(2);
      if(fixed[a])near[i].emplace_back(a,3);
      else if(fixed[b])near[i].emplace_back(b,1);
    }
  }
  int minimum=32,energy=0;
  double separation=0;
  for(int i=0;i<int(words.size());++i)for(int j=0;j<i;++j){
    int h=popcount(words[i]^words[j]);
    minimum=min(minimum,h);
    separation+=separation_penalty(h);
  }
  const double ceiling=1.045*separation+1e-9;
  for(int p:points)for(auto [q,w]:near[p])if(p<q&&bits[p]!=bits[q])energy+=w;
  auto best=bits;
  auto code=words;
  int lowest=energy;
  vector<unsigned> flip(words.size());
  Rng rng{0x225a33328d1ull};
  constexpr int steps=16000;
  for(int t=0;t<steps;++t){
    if((t&63)==0 && Clock::now()>=stop)break;
    int p=points[rng.next()%points.size()], change=0;
    for(auto [q,w]:near[p])change+=(bits[p]==bits[q]?w:-w);
    double temperature=2.8*pow(0.06,t/double(steps));
    if(change>0 && (rng.next()/4294967296.0)>=exp(-change/temperature))continue;
    for(auto [i,b]:reverse[p])flip[i]^=1u<<b;
    auto delta=check_word_edit(words,flip,minimum);
    if(delta.valid && separation+delta.delta<=ceiling){
      energy+=change;
      separation+=delta.delta;
      bits[p]^=1;
      for(auto [i,b]:reverse[p])words[i]^=1u<<b;
      if(energy<lowest)lowest=energy,best=bits,code=words;
    }
    for(auto [i,b]:reverse[p])flip[i]=0;
  }
  bits=std::move(best);
  words=std::move(code);
}
