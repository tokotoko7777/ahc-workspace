#define AHC004_TEST
#include "../problems/ahc004/main.cpp"

int main(){
    std::mt19937_64 rng(14917392);
    for(int t=0;t<70;++t) {
        int n=6+int(rng()%15);
        GenomeProblem::Board board(n,std::string(n,'A'));
        for(auto& row:board)for(char& c:row)c='A'+char(rng()%4);
        std::map<std::string,int> freq;
        for(int trial=0;trial<160;++trial){
            int r=rng()%n,c=rng()%n,axis=rng()&1,k=2+rng()%std::min(8,n-1);
            std::string word;
            for(int i=0;i<k;++i)word+=axis?board[(r+i)%n][c]:board[r][(c+i)%n];
            ++freq[word];
        }
        std::vector<Target> targets;
        for(auto& [word,k]:freq)targets.push_back(Target{word,k});
        GenomeProblem problem(n,160,targets);
        auto state=problem.make_state(board);
        for(int i=0;i<100;++i){
            int line=rng()%(2*n);
            auto move=i%3==0?problem.line_swap(state,line,line/n*n+int(rng()%n)):
                      problem.line_cycle(state,line,1+int(rng()%(n-1)));
#if defined(AHC004_EXACT_PLACEMENT) || defined(AHC004_FAST_PLACEMENT)
            if(i%7==2)move=problem.propose_move(state,rng,0.0);
#endif
            if(i%5==0)move=GenomeProblem::Move{{{int(rng()%(n*n)),'.'}}};
            if(!move)continue;
            auto old=problem.score(state);
            auto delta=problem.evaluate_move(state,*move,0.0);
            assert(delta);
            problem.apply_move(state,*move);
            auto full=problem.make_state(state.board);
            assert(state.coverage.counts==full.coverage.counts);
            assert(state.coverage.covered_weight==full.coverage.covered_weight);
            assert(state.line_code==full.line_code && state.line_empty==full.line_empty);
#ifdef AHC004_FAST_PLACEMENT
            assert(state.rotated_code==full.rotated_code && state.rotated_empty==full.rotated_empty);
            for(int k=0;k<5;++k) {
                const int q=rng()%(2*n), start=rng()%n, len=1+rng()%std::min(n,12);
                const uint64_t word=rng()&((1ULL<<(3*len))-1);
                const uint64_t mask=((1ULL<<(3*len))-1)/7;
                const uint64_t holes=state.rotated_empty[q][start]&mask;
                const uint64_t diff=state.rotated_code[q][start]^word;
                const int score=1250*(len-__builtin_popcountll((diff|(diff>>1)|(diff>>2)|holes)&mask))
                               +330*__builtin_popcountll(holes)-250*len;
                assert(score==problem.placement_quality(state,q,start,word,len));
            }
#endif
            assert(state.empty==full.empty);
            assert(std::abs(old+*delta-problem.score(full))<1e-10);
            if(i%8==0) {
                int direct=0;
                for(auto& [word,k]:freq) {
                    bool any=false;
                    for(int r=0;r<n;++r)for(int c=0;c<n;++c)for(int axis=0;axis<2;++axis){
                        bool found=true;
                        for(int j=0;j<int(word.size());++j)
                            if((axis?state.board[(r+j)%n][c]:state.board[r][(c+j)%n])!=word[j]){found=false;break;}
                        any|=found;
                    }
                    direct+=k*any;
                }
                assert(direct==state.coverage.covered_weight);
            }
            for(int j=0;j<2*n;++j){
                auto a=state.line_hits[j],b=full.line_hits[j];
                std::sort(a.begin(),a.end());std::sort(b.begin(),b.end());
                assert(a==b);
            }
        }
    }
}
