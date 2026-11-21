#pragma once
#include <array>
#include <bit>
#include <type_traits>
#include <cstdint>
#include <string>
#include <vector>

namespace ahc004 {
// n-by-n board, n in [1,20], alphabet A-H plus '.'.
// Indices 0..n-1 are rows, n..2n-1 are columns. Each position
// is mirrored at position+n so the low bits of mask >> offset index
// starts of cyclic windows, without any modulo in window matching.
template<bool Prefilter=false>
class BasicToroidalSymbolMasks {
    int n;
    uint32_t full;
    uint64_t symbol[40][9]{};
    struct Empty {};
    [[no_unique_address]] std::conditional_t<Prefilter,std::array<uint64_t,8>,Empty> has_line{};
    static int digit(char c) { return c=='.'?8:c-'A'; }
public:
    explicit BasicToroidalSymbolMasks(const std::vector<std::string>& board)
        : n(board.size()), full((uint32_t(1)<<n)-1) {
        for(int r=0;r<n;++r)for(int c=0;c<n;++c) {
            const int k=digit(board[r][c]);
            symbol[r][k]|=(uint64_t(1)<<c)|(uint64_t(1)<<(c+n));
            symbol[n+c][k]|=(uint64_t(1)<<r)|(uint64_t(1)<<(r+n));
            if constexpr(Prefilter) if(k<8) has_line[k]|=(1ULL<<r)|(1ULL<<(n+c));
        }
    }
    void set(int r,int c,char old,char fresh) {
        const int a=digit(old),b=digit(fresh);
        if(a==b)return;
        const uint64_t x=(uint64_t(1)<<c)|(uint64_t(1)<<(c+n));
        const uint64_t y=(uint64_t(1)<<r)|(uint64_t(1)<<(r+n));
        symbol[r][a]&=~x;symbol[r][b]|=x;
        symbol[n+c][a]&=~y;symbol[n+c][b]|=y;
        if constexpr(Prefilter) {
            if(a<8) {
                if(!symbol[r][a]) has_line[a]&=~(1ULL<<r);
                if(!symbol[n+c][a]) has_line[a]&=~(1ULL<<(n+c));
            }
            if(b<8) has_line[b]|=(1ULL<<r)|(1ULL<<(n+c));
        }
    }
    template<bool IncludeDots>
    uint32_t starts(int line,const char* text,int len) const {
        uint32_t ret=full;
        const uint64_t dots=IncludeDots?symbol[line][8]:0;
        for(int p=0;p<len && ret;++p)
            ret&=uint32_t((symbol[line][text[p]-'A']|dots)>>p);
        return ret;
    }
    bool present(const std::string& text) const {
        const char* s=text.data();
        const int len=text.size();
        for(int line=0;line<2*n;++line)if(starts<false>(line,s,len))return true;
        return false;
    }
    static uint32_t signature(const std::string& text) {
        uint32_t sig=0;
        for(char c:text) sig|=1U<<(c-'A');
        return sig;
    }
    bool present_prefilter(const std::string& text,uint32_t sig) const requires(Prefilter) {
        uint64_t lines=(1ULL<<(2*n))-1;
        while(sig && lines) {
            lines&=has_line[std::countr_zero(sig)];
            sig&=sig-1;
        }
        while(lines) {
            if(starts<false>(std::countr_zero(lines),text.data(),text.size())) return true;
            lines&=lines-1;
        }
        return false;
    }
    int filled_in_line(int line) const {
        return n-std::popcount(uint32_t(symbol[line][8])&full);
    }
    int dots_in_window(int line,int start,int len) const {
        return std::popcount(uint32_t(symbol[line][8]>>start)&((uint32_t(1)<<len)-1));
    }
    int dots() const {
        int cnt=0;
        for(int row=0;row<n;++row)cnt+=n-filled_in_line(row);
        return cnt;
    }
};
using ToroidalSymbolMasks=BasicToroidalSymbolMasks<>;
}  // namespace ahc004
