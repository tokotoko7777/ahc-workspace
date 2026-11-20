// Replay the experimental alternative even when the production fallback won.
#define AHC072_MIXED 1
#define main ahc072_submission_main
#include "../../main.cpp"
#undef main

int main(){
    auto deadline=Clock::now()+chrono::milliseconds(1420);
    ios::sync_with_stdio(false);cin.tie(nullptr);
    Problem p;p.read();
    (void)p.solve(false,deadline);
    (void)p.solve(true,deadline);
    Problem::PacketStats packet;
    const auto result=p.solve(true,deadline,0,0,&packet);
    if(result.empty())return 1;
    for(auto [v,k,d,h]:result)cout<<v/p.n<<' '<<v%p.n<<' '<<k<<' '<<directions[d]<<' '<<h<<'\n';
    return 0;
}
