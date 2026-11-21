#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace ahc004 {
enum class ForcedStatus { solved, stalled, contradictory, capped };
struct ForcedCompletion {
    std::vector<std::string> board;
    ForcedStatus status=ForcedStatus::stalled;
    int rounds=0, added=0;
    uint64_t checks=0;
};

// Completes only logically necessary letters on the fixed scaffold.
// The time/counter/round limits are fail-open: "capped" says nothing
// about the existence of a consistent completion.
template<class Deadline>
ForcedCompletion force_letters(std::vector<std::string> board,
                               const std::vector<std::string>& words,
                               Deadline&& expired, uint64_t check_cap=3500000,
                               int round_cap=10) {
    ForcedCompletion ans;
    ans.board=std::move(board);
    const int n=ans.board.size();
    if(n<1 || n>20)return ans;
    const uint64_t full_mask=(uint64_t(1)<<(3*n))-1;
    struct Word {
        uint64_t bits, mask;
        const std::string* text;
    };
    std::vector<Word> reads;
    reads.reserve(words.size());
    for(const auto& w:words) {
        if(w.empty() || int(w.size())>n)return ans;
        uint64_t code=0;
        for(int j=0;j<int(w.size());++j)code|=uint64_t(w[j]-'A')<<(3*j);
        reads.push_back({code,(uint64_t(1)<<(3*w.size()))-1,&w});
    }
    // Longer words usually expose contradictions earlier.
    std::stable_sort(reads.begin(),reads.end(),[](const Word& a,const Word& b) {
        return a.text->size()>b.text->size();
    });
    const auto at=[&](int line,int pos) {
        return line<n?line*n+pos:pos*n+line-n;
    };
    for(int round=0;round<round_cap;++round) {
        if(expired())break;
        uint64_t bits[800], known[800];
        for(int line=0;line<2*n;++line) {
            uint64_t row=0, mask=0;
            for(int pos=0;pos<n;++pos) {
                const int cell=at(line,pos);
                const char c=ans.board[cell/n][cell%n];
                if(c=='.')continue;
                row|=uint64_t(c-'A')<<(3*pos);
                mask|=7ULL<<(3*pos);
            }
            bits[line*n]=row;
            known[line*n]=mask;
            for(int start=1;start<n;++start){
                const int shift=3*start, inverse=3*n-shift;
                bits[line*n+start]=((row>>shift)|(row<<inverse))&full_mask;
                known[line*n+start]=((mask>>shift)|(mask<<inverse))&full_mask;
            }
        }
        char batch[400]{};
        bool all_present=true;
        for(int index=0;index<int(reads.size());++index) {
            if((index&7)==0 && expired()){
                ans.status=ForcedStatus::capped;
                return ans;
            }
            const Word& w=reads[index];
            const std::string& text=*w.text;
            const int len=text.size();
            int cells[20], positions[20], size=-1;
            char symbols[20];
            bool present=false;
            for(int p=0;p<2*n*n;++p) {
                if(ans.checks==check_cap){
                    ans.status=ForcedStatus::capped;
                    return ans;
                }
                ++ans.checks;
                const uint64_t fixed=known[p]&w.mask;
                if((bits[p]^w.bits)&fixed)continue;
                if(fixed==w.mask){present=true;size=0;break;}
                if(size==0)continue;
                const int line=p/n, start=p%n;
                if(size<0) {
                    size=0;
                    for(int j=0;j<len;++j)if(!((fixed>>(3*j))&7)) {
                        const int pos=(start+j)%n;
                        cells[size]=at(line,pos);
                        symbols[size++]=text[j];
                    }
                } else {
                    int out=0;
                    for(int k=0;k<size;++k) {
                        const int r=cells[k]/n, c=cells[k]%n;
                        if(line<n ? r!=line : c!=line-n)continue;
                        const int pos=line<n?c:r;
                        const int j=(pos+n-start)%n;
                        if(j>=len || text[j]!=symbols[k])continue;
                        positions[out++]=k;
                    }
                    for(int i=0;i<out;++i) {
                        cells[i]=cells[positions[i]];
                        symbols[i]=symbols[positions[i]];
                    }
                    size=out;
                }
            }
            if(size<0) {
                ans.status=ForcedStatus::contradictory;
                return ans;
            }
            if(!present)all_present=false;
            for(int k=0;k<size;++k) {
                const int cell=cells[k];
                if(batch[cell] && batch[cell]!=symbols[k]) {
                    ans.status=ForcedStatus::contradictory;
                    return ans;
                }
                batch[cell]=symbols[k];
            }
        }
        if(all_present) {
            ans.status=ForcedStatus::solved;
            return ans;
        }
        int added=0;
        for(int cell=0;cell<n*n;++cell)if(batch[cell]){
            ans.board[cell/n][cell%n]=batch[cell];
            ++added;
        }
        if(!added) {
            ans.status=ForcedStatus::stalled;
            return ans;
        }
        ++ans.rounds;
        ans.added+=added;
    }
    ans.status=ForcedStatus::capped;
    return ans;
}
}
