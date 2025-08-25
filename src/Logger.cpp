#include "Logger.h"
#include <Windows.h>

ElapsedTimer::ElapsedTimer()
{
    reset();
}

void ElapsedTimer::reset()
{
    _t = std::chrono::steady_clock::now();
}

double ElapsedTimer::elapsed() const
{
    double ret = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - _t).count();
    return ret;
}

double ElapsedTimer::elapsedAndReset()
{
    auto e = elapsed();
    reset();
    return e;
}

bool ElapsedTimer::isTimeout(double milliSecs) const
{
    if (milliSecs <= 0)
    {
        return false;
    }
    return elapsed() >= milliSecs;
}

void Logger::_logImpl(const std::string& log)
{
    OutputDebugStringA(log.c_str());
}

void Logger::_logImpl(const std::wstring& log)
{
    OutputDebugStringW(log.c_str());
}
