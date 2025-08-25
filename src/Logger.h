#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <format>
#include <chrono>

class ElapsedTimer
{
public:
    ElapsedTimer();
    void reset();
    double elapsed() const;
    double elapsedAndReset();
    bool isTimeout(double milliSecs) const;

private:
    std::chrono::steady_clock::time_point _t;
};

class Logger
{
public:
    template<typename... ARGS>
    static void log(std::format_string<ARGS...>&& s, ARGS&&... args)
    {
        auto originLog = std::format(s, std::forward<ARGS>(args)...);
        _logImpl(std::forward<decltype(originLog)>(originLog));
    }

private:
    static void _logImpl(const std::string& log);
    static void _logImpl(const std::wstring& log);
};

#endif
