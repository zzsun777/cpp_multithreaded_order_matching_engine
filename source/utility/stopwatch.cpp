#include "stopwatch.h"
using namespace std::chrono;

namespace utility
{

void StopWatch::start ()
{
    m_startTime = high_resolution_clock::now();
}

void StopWatch::stop ()
{
    m_endTime = high_resolution_clock::now();
}

long long StopWatch::getElapsedTimeMilliseconds ()
{
    auto ret = duration_cast<std::chrono::milliseconds>(m_endTime - m_startTime).count();
    return ret;
}

}//namespace