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
#include <time.h>

#define NANO_SECOND_MULTIPLIER  1000000

using namespace std;

class Timer
{
public:
    Timer(float time)
    {
        t = time;
    };

    //~Timer();

    void addTask(Timer *timer)
    {
        while(1) {
            const long INTERVAL_MS = t * NANO_SECOND_MULTIPLIER;
            timespec sleepValue = {0};

	    sleepValue.tv_nsec = INTERVAL_MS;
	    nanosleep(&sleepValue, NULL);
            timer->executeCb();
        }
    };

    void updateTimer(float time)
    {
        t = time;
    }

protected:
    virtual void executeCb() = 0;

private:
    float t;

};
#endif
