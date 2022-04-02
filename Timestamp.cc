#include "Timestamp.h"

Timestamp::Timestamp() : _point(std::chrono::system_clock::now())
{
}

std::string Timestamp::YearMonthDay() const
{
    char buf[20];
    int n = YearMonthDay(buf);
    buf[n] = '\0';
    return buf;
}

std::string Timestamp::HourMinSec() const
{
    char buf[20];
    int n = HourMinSec(buf);
    buf[n] = '\0';
    return buf;
}

int Timestamp::YearMonthDay(char *dst) const
{
    char save = dst[10];
    auto tp = std::chrono::time_point_cast<std::chrono::days>(_point);
    std::chrono::year_month_day ymd(tp);
    int n = sprintf(dst, FMT_DATE, static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()), static_cast<unsigned>(ymd.day()));
    dst[10] = save;
    return n;
}

int Timestamp::HourMinSec(char *dst) const
{
    char save = dst[12];
    auto tpd = std::chrono::time_point_cast<std::chrono::days>(_point);
    auto tpm = std::chrono::time_point_cast<std::chrono::milliseconds>(_point);
    std::chrono::hh_mm_ss hms(tpm - tpd);
    auto mill = hms.subseconds().count();
    if (mill < 10)
    {
        mill *= 100;
    }
    else if (mill < 100)
    {
        mill *= 10;
    }
    int n = sprintf(dst, FMT_DAY_TIME, hms.hours().count(), hms.minutes().count(), hms.seconds().count(), mill);
    dst[12] = save;
    return n;
}
