#ifndef TIMER_HEADER
#define TIMER_HEADER

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <cstring>
#include <ctime>
#include <unistd.h>
//#include <chrono>
#include <ctime>

using namespace std;
//using namespace std::chrono;

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
        //high_resolution_clock::time_point called = high_resolution_clock::now();
        clock_t called = clock();
        int flag = 0;
        while(1)
        {
            //high_resolution_clock::time_point when = high_resolution_clock::now();
            clock_t when = clock();
            //duration<double> time_span = duration_cast<duration<double>>(when - called);
            double time_span = double(when - called) / CLOCKS_PER_SEC;
            //if(time_span.count() >= t || flag == 0)
            if(time_span >= t || flag == 0)
            {
                flag = 1;
                //called = high_resolution_clock::now();
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
