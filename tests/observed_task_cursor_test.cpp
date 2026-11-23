#include "observed-task-cursor.hpp"
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
using namespace observed_task;
#define CHECK(x) do { if(!(x)) {std::cerr<<"check failed "<<__LINE__<<'\n';std::abort();} }while(false)
struct Oracle {
    enum Cell { untouched, omitted, finished };
    std::vector<Cell> tasks;
    std::vector<Request> emitted;
    std::size_t cap;
    std::uint64_t ver;
    bool waiting=false,closing=false,at_safe=false;
    Oracle(std::size_t n,std::size_t c,std::uint64_t v):tasks(n,untouched),cap(c),ver(v){}
    std::size_t head() const {
        for(std::size_t i=0;i<tasks.size();++i)if(tasks[i]==untouched)return i;
        return tasks.size();
    }
    std::size_t debt() const {
        std::size_t n=0;for(auto q:tasks)n+=q==omitted;return n;
    }
    std::optional<Request> next() const {
        if(waiting||at_safe)return {};
        if(closing)return Request{Kind::egress,0};
        const std::size_t normal=head();
        if(normal<tasks.size())return Request{Kind::normal,normal};
        for(std::size_t i=0;i<tasks.size();++i)if(tasks[i]==omitted)return Request{Kind::repair,i};
        return {};
    }
    bool done() const {return !closing&&!waiting&&debt()==0&&head()==tasks.size();}
    std::optional<Ticket> send(Request q,std::uint64_t v) {
        if(ver>v)return {};
        if(waiting){
            if(emitted.back()==q)return Ticket{emitted.size(),false};
            return {};
        }
        auto n=next();
        if(!n||q!=*n)return {};
        emitted.push_back(q);ver=v;waiting=true;
        return Ticket{emitted.size(),true};
    }
    bool response(std::uint64_t tok,std::uint64_t v,Ack a,bool auth) {
        if(v<=ver||!waiting||emitted.size()!=tok)return false;
        ver=v;
        if(a==Ack::unknown||!auth)return true;
        waiting=false;
        if(a==Ack::applied&&emitted.back().kind!=Kind::egress)
            tasks[emitted.back().task]=finished;
        return true;
    }
    bool skip(std::uint64_t v,bool cert){
        if(v<ver||!cert||waiting||closing||debt()>=cap||head()==tasks.size())return false;
        tasks[head()]=omitted;ver=v;return true;
    }
    void abort(){if(!done())closing=true;}
    bool qualify(std::uint64_t v,bool cert){
        if(!closing||at_safe||waiting||v<=ver||!cert)return false;
        ver=v;at_safe=true;return true;
    }
};
std::uint64_t calls=0;
void compare(const Cursor& a,const Oracle& b){
    CHECK(a.head()==b.head());CHECK(a.debt_size()==b.debt());CHECK(a.version()==b.ver);
    CHECK(a.done()==b.done());CHECK(a.aborted()==b.closing);CHECK(a.safe_confirmed()==b.at_safe);
    CHECK(a.next()==b.next());CHECK(a.pending_token()==(b.waiting?std::optional<std::uint64_t>(b.emitted.size()):std::nullopt));
}
void event(Cursor& a,Oracle& b,int k){
    ++calls;
    const auto v=b.ver;const auto tok=b.emitted.size();const auto peek=b.next();
    const Request g=b.waiting?b.emitted.back():peek.value_or(Request{Kind::normal,0});
    switch(k){
    case 0:CHECK(a.submit(g,v)==b.send(g,v));break;
    case 1:CHECK(a.submit(g,v?v-1:0)==b.send(g,v?v-1:0));break;
    case 2:CHECK(a.submit({g.kind,g.task+1},v+1)==b.send({g.kind,g.task+1},v+1));break;
    case 3:CHECK(a.observe(tok,v+1,Ack::applied,true)==b.response(tok,v+1,Ack::applied,true));break;
    case 4:CHECK(a.observe(tok,v+1,Ack::rejected,true)==b.response(tok,v+1,Ack::rejected,true));break;
    case 5:CHECK(a.observe(tok,v+1,Ack::unknown,true)==b.response(tok,v+1,Ack::unknown,true));break;
    case 6:CHECK(a.observe(tok,v+1,Ack::applied,false)==b.response(tok,v+1,Ack::applied,false));break;
    case 7:CHECK(a.observe(tok,v,Ack::applied,true)==b.response(tok,v,Ack::applied,true));break;
    case 8:CHECK(a.observe(tok+1,v+1,Ack::applied,true)==b.response(tok+1,v+1,Ack::applied,true));break;
    case 9:CHECK(a.defer_head(v,true)==b.skip(v,true));break;
    case 10:CHECK(a.defer_head(v+1,false)==b.skip(v+1,false));break;
    case 11:a.request_abort();b.abort();break;
    case 12:CHECK(a.confirm_safe(v+1,true)==b.qualify(v+1,true));break;
    case 13:CHECK(a.confirm_safe(v+1,false)==b.qualify(v+1,false));break;
    case 14:CHECK(a.observe(tok?tok-1:tok,v+2,Ack::rejected,true)==b.response(tok?tok-1:tok,v+2,Ack::rejected,true));break;
    }
    compare(a,b);
}
void exhaustive(const Cursor& c,const Oracle& r,int depth){
    if(!depth)return;
    for(int j=0;j<15;++j){Cursor d=c;Oracle s=r;event(d,s,j);exhaustive(d,s,depth-1);}
}
int main(){
    for(std::size_t l=0;l<=3;++l)
        for(std::size_t cap=0;cap<=2;++cap)
            exhaustive(Cursor(l,cap,7),Oracle(l,cap,7),5);
    std::mt19937 gen(0x597321u);
    for(unsigned k=0;k<20000;++k){
        const unsigned n=gen()%13,c=gen()%5;
        Cursor a(n,c,10);Oracle b(n,c,10);
        for(int l=0;l<50;++l)event(a,b,gen()%15);
    }
    std::cout<<calls<<" deterministic/exhaustive/random transition comparisons passed\n";
}
