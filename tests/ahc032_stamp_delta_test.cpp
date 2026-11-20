#define AHC032_TEST
#include "../problems/ahc032/main.cpp"

long long sum_board(const Board& board) {
  return accumulate(board.begin(),board.end(),0LL);
}
void brute(const ModStampProblem& p,Board& a,Press q,int sign) {
  for(int y=0;y<9;++y)for(int x=0;x<9;++x)
    if(q.row<=y&&y<q.row+3&&q.column<=x&&x<q.column+3){
      int id=y*9+x;
      long long add=p.stamps[q.stamp][(y-q.row)*3+x-q.column];
      a[id]=(a[id]+MODULO+sign*add)%MODULO;
    }
}
int main() {
  mt19937 rng(32232);
  ModStampProblem p;
  for(int rep=0;rep<180;++rep){
    for(auto& x:p.initial_board)x=rng()%MODULO;
    for(auto& s:p.stamps)for(auto& x:s)x=rng()%MODULO;
    auto board=p.initial_board;
    auto compare=p.initial_board;
    auto move=[&](){return Press{int(rng()%20),int(rng()%7),int(rng()%7)};};
    for(int k=0;k<120;++k){
      Press a=move(),b=move();
      int sign=rng()%2?1:-1;
      auto original=board;
      long long d=stamp(board,p,a,sign);
      d+=stamp(board,p,b,-sign);
      brute(p,compare,a,sign);
      brute(p,compare,b,-sign);
      assert(board==compare && sum_board(board)-sum_board(original)==d);
      stamp(board,p,b,sign);
      stamp(board,p,a,-sign);
      assert(board==original);
      board=compare;
    }
    if(rep>=3)continue;
    vector<Press> ops;
    board=p.initial_board;
    for(int k=0;k<81;++k){ops.push_back(move());brute(p,board,ops.back(),1);}
    long long score=sum_board(board);
    repair(p,board,ops,Clock::time_point::max());
    compare=p.initial_board;
    for(auto op:ops)brute(p,compare,op,1);
    assert(board==compare && score<=sum_board(board) && ops.size()==81);
  }
}
