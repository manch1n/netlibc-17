#include <bits/stdc++.h>
#include "Observer.hpp"
#include "Latch.hpp"
#include "Singleton.hpp"
#include "ThreadPool.h"
#include "Timestamp.h"
#include "Logger.h"
using namespace std;

class printer : public Observer
{
public:
    printer()
    {
        counter++;
    }
    virtual void update() override
    {
        printf("I am:%d\n", counter.load());
    }
    static inline atomic<int> counter;
};

class Sample
{
public:
    Sample()
    {
        cout << "default constrcutor" << endl;
    }
    Sample(int n) : _n(n)
    {
        cout << "recv param" << endl;
    }
    int _n;
};

template <typename... ARGS>
void vaprinter(const char *fmt, const ARGS &...args)
{
    printf(fmt, args...);
}

int main(int argc, char **argv)
{
    { // both exist
        Observable obsrv;
        auto p1 = make_shared<printer>();
        p1->observe(&obsrv);
        obsrv.notify();
    }
    { // observer didnt exist
        Observable obsrv;
        {
            auto p1 = make_shared<printer>();
            p1->observe(&obsrv);
        }
        obsrv.notify();
    }
    {
        // obsrvable didnt exist
        auto p1 = make_shared<printer>();
        {
            Observable obsrv;
            p1->observe(&obsrv);
        }
    }
    { // main thread wait
        CountDownLatch latch(4);
        auto threadFunc = [&latch]()
        {
            cout << "myid: " << this_thread::get_id() << endl;
            latch.CountDown();
        };
        vector<thread> threads;
        for (int i = 0; i < 4; ++i)
        {
            threads.emplace_back(threadFunc);
        }
        latch.wait();
        printf("main\n");
        for_each(threads.begin(), threads.end(), [](thread &th)
                 { th.join(); });
    }
    {
        Singleton<Sample> single;
        //      auto instance = single.instance();
        //      cout << instance._n << endl;
        single.init(1);
    }
    { // threadpoll
        auto func = []()
        {
            cout << "i am: " << this_thread::get_id() << endl;
        };
        ThreadPool pool(6);
        pool.pushTask(func);
        pool.stop();
    }
    { // FILE vs fstream
        // const char *str = "hello worlddjasljdlasjd\n";
        // {
        //     char buf[8192];
        //     FILE *fp = fopen("test2.txt", "w");
        //     setbuffer(fp, buf, sizeof(buf));
        //     auto start = chrono::high_resolution_clock::now();

        //     for (int i = 0; i < 10000000; ++i)
        //     {
        //         fwrite_unlocked(str, 1, 24, fp);
        //     }
        //     fflush(fp);
        //     fclose(fp);
        //     auto finish = chrono::high_resolution_clock::now();
        //     chrono::duration<double, std::milli> d = finish - start;
        //     cout << d.count() << endl;
        // }
        // {
        //     std::ofstream oftrm;
        //     char buf[8192];
        //     oftrm.rdbuf()->pubsetbuf(buf, sizeof(buf));
        //     oftrm.open("test1.txt", ios_base::out);
        //     auto start = chrono::high_resolution_clock::now();
        //     for (int i = 0; i < 10000000; ++i)
        //     {
        //         oftrm << str;
        //     }
        //     oftrm.flush();
        //     oftrm.close();
        //     auto finish = chrono::high_resolution_clock::now();
        //     chrono::duration<double, std::milli> d = finish - start;

        //     cout << d.count() << endl;
        // }
        // ostringstream vs sprintf
        {
            char buf[4096];
            auto start = chrono::high_resolution_clock::now();
            for (int i = 0; i < 10000; ++i)
            {
                snprintf(buf, sizeof(buf), "hello world %d %d %d\n", 1, 2, 3);
            }
            auto finish = chrono::high_resolution_clock::now();
            chrono::duration<double, std::milli> d = finish - start;
            cout << d.count() << endl;
        }
        {
            auto start = chrono::high_resolution_clock::now();
            ostringstream ostrm;
            for (int i = 0; i < 10000; ++i)
            {
                ostrm << "hello world " << 1 << " " << 2 << " " << 3 << "\n";
                ostrm.str();
            }
            auto finish = chrono::high_resolution_clock::now();
            chrono::duration<double, std::milli> d = finish - start;
            cout << d.count() << endl;
        }
        auto cstdioprinter = [](const char *fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            vprintf(fmt, args);
            va_end(args);
        };
        cstdioprinter("abc:%d\n", 1);
        {
            using namespace chrono;
            auto tp = time_point_cast<days>(system_clock::now());
            year_month_day ymd(tp);
            cout << static_cast<int>(ymd.year()) << " " << static_cast<unsigned>(ymd.month()) << " " << static_cast<unsigned>(ymd.day()) << endl;
            // auto tp2 = duration_cast<milliseconds>(system_clock::now());
            auto tp2 = time_point_cast<milliseconds>(system_clock::now());
            auto res = tp2 - tp;
            hh_mm_ss hms(tp2 - tp);
            cout << hms.hours().count() << " " << hms.minutes().count() << " " << hms.seconds().count() << " " << hms.subseconds().count() << endl;
        }
        {
            Timestamp stamp;
            cout << stamp.YearMonthDay() << endl;
            cout << stamp.HourMinSec() << endl;
            char buf[40] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            stamp.HourMinSec(buf);
            vaprinter("1:%d\n", 1);
        }
        {
            enableFileLog();
            auto start = chrono::high_resolution_clock::now();
            for (int i = 0; i < 10000; ++i)
            {
                log_info("he%d%do asyn log", 1, 1);
            }
            auto finish = chrono::high_resolution_clock::now();
            chrono::duration<double, std::milli> result = finish - start;
            cout << "result: " << result.count() / 1000 << endl;
        }
    }
    return 0;
}