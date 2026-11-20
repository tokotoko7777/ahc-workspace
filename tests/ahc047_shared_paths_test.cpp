#define AHC047_TEST
#include "../problems/ahc047/main.cpp"

int main(){
  mt19937 rng(470448);
  for(int t=0;t<500;++t){
    Problem p;
    p.L=1000000;
    for(int j=0;j<36;++j){
      string w(6+rng()%7,'a');
      for(char& c:w)c+=rng()%6;
      p.word.emplace_back(w,1+rng()%17000);
    }
    for(int type=0;type<2;++type){
      auto m=p.shared_paths(type);
      assert(m.label.size()==N);
      for(int i=0;i<6;++i)assert(m.label.find('a'+i)!=string::npos);
      for(auto row:m.row){
        assert(accumulate(row.begin(),row.end(),0)==100);
        assert(*min_element(row.begin(),row.end())>=0);
      }
      for(int j=0;j<10;++j){
        auto& word=p.word[j].first;
        array<char,N> live{},next{};
        for(int i=0;i<N;++i)live[i]=m.label[i]==word[0];
        for(int k=1;k<int(word.size());++k){
          next.fill(false);
          for(int a=0;a<N;++a)if(live[a])
            for(int b=0;b<N;++b)if(m.label[b]==word[k]&&m.row[a][b]>0)next[b]=true;
          live=next;
        }
        assert(any_of(live.begin(),live.end(),[](char c){return c!=0;}));
      }
    }
  }
}
