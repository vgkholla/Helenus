#ifndef TIMER_HEADER
#define TIMER_HEADER

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <ctime>

using namespace std;

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
        clock_t called = clock();
        int flag = 0;
        while(1)
        {
            clock_t when = clock();
            double time_span = double(when - called) / CLOCKS_PER_SEC;
            if(time_span >= t || flag == 0)
            {
                flag = 1;
                called = clock();
                timer->executeCb();
            }
        }
    };

    void updateTimer(int time)
    {
        t = time;
    }

protected:
    virtual void executeCb() = 0;

private:
    int t;

};
#endif
