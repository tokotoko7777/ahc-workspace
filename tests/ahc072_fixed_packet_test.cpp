#include "../problems/ahc072/experiments/fixed_packet_v3/packet_router.hpp"
#include <array>
#include <cassert>
#include <deque>
#include <map>
#include <random>
#include <vector>
using namespace std;
using namespace ahc072_packet;

struct Scenario {
    int w,h;
    vector<int> homes;
    vector<char> wall;
    vector<vector<int>> floor;
    int ray(int p,int d,int size)const{
        constexpr int dy[]={-1,1,0,0},dx[]={0,0,-1,1};
        int y=p/w,x=p%w;
        for(int i=0;i<size;++i){
            y+=dy[d];x+=dx[d];
            if(y<0||y>=h||x<0||x>=w||wall[y*w+x])return -1;
        }
        return y*w+x;
    }
    optional<vector<Jump>> model(int p,const vector<int>& packet)const{
        vector<Support> support;
        for(const auto& st:floor)support.push_back({int(st.size()),st.empty()?-1:st.back()});
        return shortest(p,packet,support,homes,[&](int u,int d,int k){return ray(u,d,k);});
    }
    void pop(vector<vector<int>>& board,int at)const{
        auto& s=board[at];
        while(!s.empty() && s.back()==homes[at])s.pop_back();
    }
    optional<int> literal(int pos,const vector<int>& packet)const{
        using State=pair<int,vector<vector<int>>>;
        auto initial=floor;
        initial[pos].insert(initial[pos].end(),packet.begin(),packet.end());
        State origin{pos,initial};
        map<State,int> length{{origin,0}};
        deque<State> queue{origin};
        while(!queue.empty()){
            auto [p,board]=queue.front();queue.pop_front();
            int dist=length[{p,board}],base=floor[p].size();
            for(int d=0;d<4;++d)for(int jump=1;jump<=base+1;++jump){
                int q=ray(p,d,jump);
                if(q<0)break;
                auto next=board;
                int size=next[p].size()-base;
                if(int(next[q].size())+size>8)continue;
                assert(size>0);
                for(int k=0;k<size;++k){next[q].push_back(next[p].back());next[p].pop_back();}
                pop(next,p);pop(next,q);
                assert(next[p]==floor[p]); // both ends are processed
                if(next==floor)return dist+1;
                if(length.emplace(State{q,next},dist+1).second)queue.emplace_back(q,move(next));
            }
        }
        return nullopt;
    }
    void replay(int pos,const vector<int>& packet,const vector<Jump>& path)const{
        auto board=floor;
        board[pos].insert(board[pos].end(),packet.begin(),packet.end());
        for(auto [p,k,d,z]:path){
            assert(p==pos && k==int(floor[p].size()) && z<=k+1);
            int q=ray(p,d,z);
            assert(q>=0 && board[q].size()+board[p].size()-k<=8);
            while(int(board[p].size())>k){board[q].push_back(board[p].back());board[p].pop_back();}
            pop(board,p);pop(board,q);assert(board[p]==floor[p]);
            pos=q;
        }
        assert(board==floor);
    }
};

int main(){
    Scenario parity{4,1,{-1,-1,0,1},vector<char>(4),vector<vector<int>>(4)};
    assert(!parity.model(0,{0,1}) && !parity.literal(0,{0,1}));
    parity.floor[0]={2};
    auto fix=parity.model(0,{0,1});
    assert(fix && fix->size()==2 && fix->front().length==2 && parity.literal(0,{0,1})==2);
    parity.replay(0,{0,1},*fix);
    Scenario cap{3,1,{-1,0,1},vector<char>(3),{{},{2,2,2,2,2,2,2},{}}};
    assert(!cap.model(0,{0,1}) && !cap.literal(0,{0,1}));
    mt19937 rng(720013);
    int reachable=0,unreachable=0, tested=0;
    while(tested<700){
        Scenario s{3,3,vector<int>(9,-1),vector<char>(9),vector<vector<int>>(9)};
        vector<int> positions(9);iota(positions.begin(),positions.end(),0);
        shuffle(positions.begin(),positions.end(),rng);
        for(int i=0;i<3;++i)s.homes[positions[i]]=i;
        for(int i=4;i<9;++i)s.wall[positions[i]]=rng()%5==0;
        for(int p=0;p<9;++p)if(!s.wall[p]){
            for(int k=rng()%8;k>0;--k)s.floor[p].push_back(rng()%3);
            s.pop(s.floor,p); // stable by construction
        }
        int p=positions[3];
        vector<int> packet;
        for(int k=1+rng()%(8-s.floor[p].size());k>0;--k)packet.push_back(rng()%3);
        auto actual=s.model(p,packet);
        auto expected=s.literal(p,packet);
        assert(bool(actual)==bool(expected));
        if(actual){
            assert(int(actual->size())==*expected);s.replay(p,packet,*actual);++reachable;
        }else ++unreachable;
        ++tested;
    }
    assert(reachable>100 && unreachable>100);
}
