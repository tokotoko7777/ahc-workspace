#include "../library/calendar-interval-packing.hpp"
#include <iostream>

int main() {
    CalendarIntervalPacking calendar(5);
    calendar.add(0,1,2,4);
    calendar.add(1,2,5,11);
    calendar.add(2,4,5,6);
    std::vector<unsigned char> blackout{0,0,0,1,0,0};
    auto plan=calendar.best(blackout,[](int){return true;});
    std::cout<<plan.value<<'\n'; // 10, by using jobs 0 and 2
    return plan.value==10 ? 0 : 1;
}
