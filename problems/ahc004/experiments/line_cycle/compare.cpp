#define AHC004_TEST
#include "../../main.cpp"

int main(int argc,char** argv){
    if(argc!=4)return 2;
    std::ifstream inp(argv[1]),board_in(argv[2]);
    std::string mode=argv[3];
    if(mode!="control"&&mode!="mixed")return 3;
    int n,m;inp>>n>>m;
    std::map<std::string,int> w;
    for(int i=0;i<m;++i){std::string s;inp>>s;++w[s];}
    std::vector<Target> t;
    for(auto& [s,k]:w)t.push_back(Target{s,k});
    GenomeProblem problem(n,m,t);
    GenomeProblem::Board b(n);
    for(auto& r:b)board_in>>r;
    if(!inp||!board_in)return 4;
    auto state=problem.make_state(b),best=state;
    std::mt19937_64 rng(0x87012fc3);
    using Clock=std::chrono::steady_clock;
    auto begin=Clock::now(),deadline=begin+std::chrono::milliseconds(105);
    int tries=0,accept=0,changed_kind=0,tried_kind=0;
    double start=problem.score(state);
    while(Clock::now()<deadline) {
        std::optional<GenomeProblem::Move> move;
        // Freeze a 1:1 mix before inspecting any of the new cases.
        const bool rearrange=mode=="mixed"&&(rng()&1);
        if(rearrange) {
            int line=int(rng()%(2*n));
            if(rng()&1)move=problem.line_cycle(state,line,1+int(rng()%(n-1)));
            else move=problem.line_swap(state,line,line/n*n+int(rng()%n));
            ++tried_kind;
        } else move=problem.propose_move(state,rng,0.0);
        ++tries;
        if(!move)continue;
        auto gain=problem.evaluate_move(state,*move,0.0);
        // Use the same short final-phase anneal in both arms. Best board is emitted.
        if(!gain)continue;
        double progress=std::chrono::duration<double,std::milli>(Clock::now()-begin).count()/105.0;
        double temp=max(0.03,0.55*pow(0.03/0.55,progress));
        if(*gain<0 && std::generate_canonical<double,53>(rng)>=std::exp(*gain/temp))continue;
        problem.apply_move(state,*move);
        ++accept;changed_kind+=rearrange;
        if(problem.score(state)>problem.score(best))best=state;
    }
    for(const auto& r:best.board)std::cout<<r<<'\n';
    std::cerr<<"before="<<start<<" after="<<problem.score(best)
             <<" trials="<<tries<<" accepted="<<accept
             <<" line_trials="<<tried_kind<<" line_accept="<<changed_kind<<'\n';
}
