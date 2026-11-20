#define AHC004_TEST
#include "../../main.cpp"

int main() {
    vector<Target> words;
    mt19937_64 in(0x004123);
    set<string> unique;
    while(words.size()<260){
        string w(7+in()%5,'A');
        for(char& c:w)c=char('A'+in()%8);
        if(unique.insert(w).second)words.push_back({w,1});
    }
    GenomeProblem p(20,260,words);
    GenomeProblem::Board board(20,string(20,'A'));
    for(auto& row:board)for(char& c:row)c=char('A'+in()%8);
    const auto state=p.make_state(board);
    mt19937_64 rng(882244);
    uint64_t hash=5381;
    Timer timer;
    constexpr int steps=25000;
    for(int i=0;i<steps;++i){
        auto move=p.propose_move(state,rng,0);
        if(!move)continue;
        for(auto [cell,c]:move->changes)hash=hash*131+cell*97+int(c);
        auto delta=p.evaluate_move(state,*move,-INFINITY);
        hash=hash*131+uint64_t(int(*delta)+1000);
    }
    double time=timer.elapsed_ms();
    cout<<steps<<' '<<hash<<' '<<rng()<<' '<<fixed<<setprecision(3)<<time<<" ms\n";
}
