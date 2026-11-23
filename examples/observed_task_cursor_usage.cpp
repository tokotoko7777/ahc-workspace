#include "observed-task-cursor.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>
using namespace observed_task;
#define REQUIRE(x) do { if(!(x))std::abort(); }while(false)
int main(){
    Cursor work(3,1,10);
    // A deployment has three ordered subtasks. Stage 0 cannot run now:
    // operator certifies stages 1/2 don't consume it and the wire can be revisited safely.
    REQUIRE(!work.defer_head(10,false));
    REQUIRE(work.defer_head(10,true));
    auto install=work.submit(*work.next(),10);
    REQUIRE(install&&install->fresh);
    REQUIRE(work.submit({Kind::normal,1},10)==(Ticket{install->token,false}));
    // An externally caused sensor change isn't proof our request was applied.
    REQUIRE(work.observe(install->token,11,Ack::applied,false));
    REQUIRE(!work.next()&&!work.confirm_safe(12,true));
    REQUIRE(work.observe(install->token,12,Ack::rejected,true));
    auto replacement=work.submit(*work.next(),12);
    REQUIRE(replacement&&replacement->fresh&&replacement->token!=install->token);
    REQUIRE(!work.observe(install->token,13,Ack::applied,true));
    REQUIRE(work.observe(replacement->token,13,Ack::applied,true));
    auto finish=work.submit(*work.next(),13);
    REQUIRE(finish&&work.observe(finish->token,14,Ack::applied,true));
    REQUIRE(work.next()==(Request{Kind::repair,0}));
    auto repair=work.submit(*work.next(),14);
    REQUIRE(repair&&work.observe(repair->token,15,Ack::applied,true));
    REQUIRE(work.done());
    Cursor halted(1,0,10);
    auto bad=halted.submit(*halted.next(),10);
    REQUIRE(halted.observe(bad->token,11,Ack::unknown,true));
    halted.request_abort(); // no independent action can overtake an in-flight effect
    REQUIRE(!halted.next()&&!halted.confirm_safe(12,true));
    REQUIRE(halted.observe(bad->token,12,Ack::rejected,true));
    REQUIRE(halted.next()==(Request{Kind::egress,0}));
    auto leave=halted.submit(*halted.next(),12);
    REQUIRE(halted.observe(leave->token,13,Ack::applied,true));
    REQUIRE(!halted.safe_confirmed()&&!halted.confirm_safe(13,true));
    REQUIRE(halted.confirm_safe(14,true)&&!halted.next()&&!halted.done());
    std::cout<<"veto, deferred work, attribution, retry and certified safe exit passed\n";
}
