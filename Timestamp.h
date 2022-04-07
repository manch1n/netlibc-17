#pragma once

#include <chrono>
#include <string>

class Timestamp
{
public:
    Timestamp();
    std::string YearMonthDay() const;
    std::string HourMinSec() const;
    // dst size should have at least 11 11 is null
    int YearMonthDay(char *dst) const;
    // dst size should have at least 13 13 is null
    int HourMinSec(char *dst) const;
    static Timestamp now();

    uint64_t getNanoseconds() const;
    static constexpr unsigned DAY_TIME_LEN = 12;
    bool operator<(const Timestamp &right) const;
    bool operator>(const Timestamp &right) const
    {
        return this->_point > right._point;
    }
    Timestamp operator+(double seconds);

private:
    static constexpr char FMT_DATE[] = "%4d-%02u-%02u";
    static constexpr char FMT_DAY_TIME[] = "%02ld:%02ld:%02ld.%-3ld";

    std::chrono::time_point<std::chrono::system_clock> _point;
};