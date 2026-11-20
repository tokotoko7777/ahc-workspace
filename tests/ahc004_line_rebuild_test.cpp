#define AHC004_TEST
#define AHC004_FAST_PLACEMENT
#define AHC004_LINE_REBUILD
#include "../problems/ahc004/main.cpp"

int main(){
    const vector<Target> targets{
        {"ABCDE",2},{"CDEFG",3},{"GHHH",2},{"FGH",1},
        {"HHAB",4},{"ACBA",1},{"BBHB",1}
    };
    GenomeProblem p(20,14,targets);
    GenomeProblem::Board empty(20,string(20,'.'));
    auto s=p.make_state(empty);
    Timer timer;
    GenomeProblem::RebuildStats stats;
    assert(p.rebuild_line(s,0,timer,1000,stats));
    assert(stats.checked>1 && stats.accepted==1 && s.coverage.covered_weight>=5);
    auto copy=p.make_state(s.board);
    assert(copy.coverage.counts==s.coverage.counts);
    assert(copy.rotated_code==s.rotated_code && copy.rotated_empty==s.rotated_empty);
    for(int r=1;r<20;++r)assert(s.board[r]==empty[r]);

    mt19937 rng(41004);
    for(int it=0;it<12;++it){
        auto board=empty;
        for(auto& row:board)for(auto& cell:row)if(rng()%3)cell=char('A'+rng()%8);
        // Force a cyclic orthogonal occurrence through the chosen cell.
        for(int j=0;j<5;++j)board[(18+j)%20][7]="ABCDE"[j];
        const int row=it%2?0:27;
        auto orig=p.make_state(board),blank=orig;
        auto rewards=p.orthogonal_rewards(orig,row,blank);
        for(int k=0;k<20;++k)for(int c=0;c<8;++c){
            const int cell=p.line_cells[row][k],cross=row<20?20+cell%20:cell/20;
            auto trial=blank.board;
            trial[cell/20][cell%20]=char('A'+c);
            int expected=0;
            for(int id=0;id<int(targets.size());++id)if(!blank.coverage.counts[id]){
                bool covered=false;
                for(int start=0;start<20;++start){
                    bool hit=true;
                    for(int j=0;j<int(targets[id].text.size());++j){
                        int q=(start+j)%20;
                        char letter=cross<20?trial[cross][q]:trial[q][cross-20];
                        hit&=letter==targets[id].text[j];
                    }
                    covered|=hit;
                }
                if(covered)expected+=targets[id].frequency;
            }
            assert(expected==rewards[k][c]);
        }
        Timer t;
        double before=p.score(orig);
        p.rebuild_line(orig,row,t,1000,stats,it%3);
        assert(p.score(orig)>=before);
        const auto full=p.make_state(orig.board);
        assert(full.coverage.counts==orig.coverage.counts && p.score(full)==p.score(orig));
        assert(full.rotated_code==orig.rotated_code && full.rotated_empty==orig.rotated_empty);
    }
}
