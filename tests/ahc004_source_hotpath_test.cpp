#define AHC004_TEST
#define AHC004_FAST_PLACEMENT
#define AHC004_EXACT_PLACEMENT
#define AHC004_SOURCE_HOTPATH
#include "../problems/ahc004/main.cpp"

int main(){
    mt19937_64 rng(20261121);
    GenomeProblem::Board board(20,string(20,'.'));
    for(auto& row:board)for(char& c:row)c=char('A'+rng()%8);
    vector<Target> targets;
    for(int i=0;i<150;++i){
        int line=rng()%40,at=rng()%20;
        string pattern;
        for(int k=0;k<(i%7)+4;++k){
            int a=(at+k)%20;pattern+=line<20?board[line][a]:board[a][line-20];
        }
        targets.push_back({pattern,1+i%3});
    }
    GenomeProblem p(20,300,targets);
    auto s=p.make_state(board);
    vector<int> old,packed;
    auto verify=[&](){
        for(int r=0;r<20;++r)assert(memcmp(s.packed_board+20*r,s.board[r].data(),20)==0);
        for(int line=0;line<40;++line){
            p.scan_line(s.board,line,old);
            p.scan_flat(s.packed_board,line,packed);
            assert(packed==old);
            auto cache=s.line_hits[line];ranges::sort(cache);ranges::sort(packed);
            assert(packed==cache);
        }
    };
    verify();
    for(int i=0;i<180;++i){
        GenomeProblem::Move mv;
        if(i%5==0) {
            auto cyclic=p.line_cycle(s,i%40,1+rng()%19);
            if(!cyclic)continue;
            mv=move(*cyclic);
        }else if(i%7==0){
            auto sw=p.line_swap(s,i%20,(i+1)%20);
            if(!sw)continue;
            mv=move(*sw);
        }else for(int k=0;k<i%8+1;++k){
            const int v=rng()%400;
            const int c=rng()%9;
            mv.changes.emplace_back(v,c==8?'.':char('A'+c));
        }
        auto naive=s.board;
        for(auto [k,c]:mv.changes)naive[k/20][k%20]=c;
        const auto want=p.make_state(naive);
        assert(*p.evaluate_move(s,mv,-INFINITY)==p.score(want)-p.score(s));
        p.apply_move(s,mv);
        assert(s.board==naive && s.coverage.counts==want.coverage.counts);
        assert(s.rotated_code==want.rotated_code && s.rotated_empty==want.rotated_empty);
        verify();
    }
}
