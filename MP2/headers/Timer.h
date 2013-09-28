#ifndef TIMER_HEADER
#define TIMER_HEADER

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

class Timer
{
public:
    Timer(int time)
    {
        t = time;
    };

    //~Timer();

    void addTask(Timer *timer)
    {
        high_resolution_clock::time_point called = high_resolution_clock::now();
        int flag = 0;
        while(1)
        {
            high_resolution_clock::time_point when = high_resolution_clock::now();
            duration<double> time_span = duration_cast<duration<double>>(when - called);
            if(time_span.count() >= t || flag == 0)
            {
                flag = 1;
                called = high_resolution_clock::now();
                timer->executeCb();
            }
        }
    };

protected:
    virtual void executeCb() = 0;

private:
    int t;

};
#endif
