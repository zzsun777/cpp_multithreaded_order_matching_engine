#ifndef __STOP_WATCH_H__
#define __STOP_WATCH_H__

#include <chrono>

namespace utility
{

class StopWatch
{
    public :

        void start();
        void stop();
        long long getElapsedTimeMilliseconds();

    private:

        std::chrono::high_resolution_clock::time_point m_startTime;
        std::chrono::high_resolution_clock::time_point m_endTime;
};

}//namespace
#endif