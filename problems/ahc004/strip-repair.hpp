#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ahc004::strip {
using Board=std::vector<std::string>;
struct Choice {
  int a=-1,b=-1,quality=0,exact=0,one=0,two=0;
  int cells=0;
};
class Evidence {
  struct Placement {
    uint32_t known, wrong;
    uint8_t line, start, holes;
  };
  const std::vector<std::string>& words;
  const std::vector<int>& weights;
  int n;
  std::vector<std::vector<Placement>> support;

  std::pair<uint32_t,int> after(const Placement& p,int a,int b) const {
    uint32_t clear=0;
    for(int x:{a,b})if(x>=0) {
      // A read wholly on the erased strip is entirely unconstrained.
      if(x==p.line)return {0,99};
      if((x<n)!=(p.line<n))clear|=1U<<(x%n);
    }
    if(p.wrong&~clear)return {0,99};
    return {clear, p.holes+std::popcount(p.known&clear)};
  }
public:
  template<class Deadline>
  Evidence(const Board& board,const std::vector<std::string>& texts,
           const std::vector<int>& w,Deadline&& stop):
      words(texts),weights(w),n(board.size()),support(words.size()) {
    if(n>20 || n<3)return;
    const uint64_t full=(1ULL<<(3*n))-1;
    struct Window {uint64_t letters,mask;uint32_t known;};
    Window rotated[40][20]{};
    const uint32_t small=(1U<<n)-1;
    for(int l=0;l<2*n;++l) {
      uint64_t code=0,mask=0;
      uint32_t known=0;
      for(int p=0;p<n;++p) {
        const char c=l<n?board[l][p]:board[p][l-n];
        if(c=='.')continue;
        code|=uint64_t(c-'A')<<(3*p);
        mask|=7ULL<<(3*p);
        known|=1U<<p;
      }
      for(int s=0;s<n;++s)
        rotated[l][s]={((code>>(3*s))|(code<<(3*(n-s))))&full,
                       ((mask>>(3*s))|(mask<<(3*(n-s))))&full,
                       ((known>>s)|(known<<(n-s)))&small};
    }
    const auto unrotate=[&](uint32_t m,int s) {
      return ((m<<s)|(m>>(n-s)))&small;
    };
    for(int i=0;i<int(words.size());++i) {
      if((i&7)==0 && stop())return;
      const int len=words[i].size();
      // Keep at least three fixed anchors for evidence.
      if(len<3||len>n)continue;
      const int max_holes=std::min(2,len-3);
      const uint32_t prefix=(1U<<len)-1;
      const uint64_t word_mask=(1ULL<<(3*len))-1;
      uint64_t code=0;
      for(int j=0;j<len;++j)code|=uint64_t(words[i][j]-'A')<<(3*j);
      for(int l=0;l<2*n;++l)for(int s=0;s<n;++s) {
        const auto& win=rotated[l][s];
        const uint32_t known=win.known&prefix;
        const int holes=len-std::popcount(known);
        if(holes>max_holes)continue;
        uint64_t diff=(win.letters^code)&win.mask&word_mask;
        uint32_t wrong=0;
        while(diff && holes+std::popcount(wrong)<=max_holes) {
          const int pos=std::countr_zero(diff)/3;
          wrong|=1U<<pos;
          diff&=~(7ULL<<(3*pos));
        }
        if(diff || holes+std::popcount(wrong)>max_holes)continue;
        support[i].push_back({unrotate(known,s),unrotate(wrong,s),
                   uint8_t(l),uint8_t(s),uint8_t(holes)});
      }
    }
    complete=true;
  }
  bool complete=false;

  Choice assess(int a=-1,int b=-1) const {
    Choice c{a,b};
    for(int i=0;i<int(support.size());++i) {
      int holes=3;
      for(const auto& p:support[i]) {
        holes=std::min(holes,after(p,a,b).second);
        if(holes==0)break;
      }
      // A three/four-character read requires at least three retained anchors.
      if(int(words[i].size())-holes<3)continue;
      const int w=weights[i];
      c.exact+=w*(holes==0);c.one+=w*(holes==1);c.two+=w*(holes==2);
    }
    return c;
  }
  Board blank(Board board,const Choice& c) const {
    for(int line:{c.a,c.b})if(line>=0)
      for(int p=0;p<n;++p)
        (line<n ? board[line][p] : board[p][line-n])='.';
    return board;
  }
  std::pair<Board,int> seed(Board board,const Choice& c) const {
    std::array<std::array<int,8>,400> votes{};
    for(int i=0;i<int(support.size());++i) {
      int best=3;
      std::array<std::pair<int,int>,2> shared;
      int count=0;
      for(const auto& p:support[i]) {
        const auto [mask,holes]=after(p,c.a,c.b);
        if(holes>best || holes>=3 || int(words[i].size())-holes<3)continue;
        if(holes==0){best=0;count=0;break;}
        std::array<std::pair<int,int>,2> proposed{};
        int used=0;
        for(int j=0;j<int(words[i].size());++j) {
          int pos=(p.start+j)%n;
          if(!(mask>>pos&1))continue;
          const int cell=p.line<n?p.line*n+pos:pos*n+p.line-n;
          proposed[used++]={cell,words[i][j]-'A'};
        }
        if(holes<best){best=holes;shared=proposed;count=used;}
        else {
          int kept=0;
          for(int j=0;j<count;++j)
            for(int k=0;k<used;++k)if(shared[j]==proposed[k]){shared[kept++]=shared[j];break;}
          count=kept;
        }
      }
      for(int j=0;j<count;++j) {
        const auto [cell,symbol]=shared[j];
        votes[cell][symbol]+=(best==1?3:1)*weights[i];
      }
    }
    board=blank(std::move(board),c);
    int filled=0;
    for(int cell=0;cell<n*n;++cell) {
      auto& v=votes[cell];
      const int x=std::max_element(v.begin(),v.end())-v.begin();
      int next=0;
      for(int j=0;j<8;++j)if(j!=x)next=std::max(next,v[j]);
      if(v[x]>=3 && v[x]>=next*2+1) {
        board[cell/n][cell%n]=char('A'+x);
        ++filled;
      }
    }
    return {std::move(board),filled};
  }

  template<class Deadline>
  std::vector<Choice> rank(const Board& board,Deadline&& stop)const {
    auto base=assess();
    auto evaluate=[&](int a,int b) {
      auto c=assess(a,b);
      for(int x:{a,b})if(x>=0)for(int p=0;p<n;++p)
        c.cells+=(x<n?board[x][p]:board[p][x-n])!='.';
      c.quality=10*(c.exact-base.exact)+9*(c.one-base.one)+5*(c.two-base.two)-c.cells/3;
      return c;
    };
    auto better=[](const Choice& a,const Choice& b) {
      return std::tie(a.quality,a.exact,a.one,a.two)>std::tie(b.quality,b.exact,b.one,b.two);
    };
    std::vector<Choice> single,pair;
    for(int a=0;a<2*n;++a) {
      if(stop())break;
      auto c=evaluate(a,-1);
      if(c.cells)single.push_back(c);
    }
    std::stable_sort(single.begin(),single.end(),better);
    if(single.empty())return {};
    for(int j=0;j<std::min<int>(6,single.size());++j)
      for(int k=0;k<j;++k) {
        if(stop())break;
        pair.push_back(evaluate(single[k].a,single[j].a));
      }
    std::stable_sort(pair.begin(),pair.end(),better);
    std::vector<Choice> ret{single.front()};
    if(!pair.empty())ret.push_back(pair.front());
    if(single.size()>1)ret.push_back(single[1]);
    return ret;
  }
};
}
