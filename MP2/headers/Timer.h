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

#include <boost/asio.hpp>
#include <boost/bind.hpp>

using namespace std;

class Timer
{
public:
    
    Timer(boost::asio::io_service& io_service, long int interval) : _timer(io_service) {

        _interval = interval;
        // now schedule the first timer.
        /* _timer.expires_from_now(boost::posix_time::milliseconds(_interval)); // runs every interval ms
        _timer.async_wait(boost::bind(&Timer::handleTimeOut, this, boost::asio::placeholders::error));*/
    }


    void handleTimeOut(boost::system::error_code const& cError) {
        if (cError.value() == boost::asio::error::operation_aborted)
            return;

        if (cError && cError.value() != boost::asio::error::operation_aborted)
            return; // throw an exception?

        cout << "timer expired" << endl;

        this->executeCb();
        // Schedule the timer again...
        _timer.expires_from_now(boost::posix_time::milliseconds(_interval)); // runs every interval ms
        _timer.async_wait(boost::bind(&Timer::handleTimeOut, this, boost::asio::placeholders::error));

    }

   

    void addTask()
    {
        // now schedule the first timer.
        _timer.expires_from_now(boost::posix_time::milliseconds(_interval)); // runs every interval ms
        _timer.async_wait(boost::bind(&Timer::handleTimeOut, this, boost::asio::placeholders::error));   
    };

    /*void updateTimer(float time)
    {
        //t = time;
    }*/

protected:
    virtual void executeCb() = 0;

private:
    boost::asio::deadline_timer _timer;
    long int _interval;


};
#endif
