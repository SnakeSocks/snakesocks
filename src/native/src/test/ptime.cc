#include <sys/time.h>
#include <stdio.h>
#include <functional>
#include "../lib/threadPool.hpp"
#include <thread>

long test(std::function<void()> f)
{
    timespec ts;
    timespec ts2;

    clock_gettime(CLOCK_REALTIME, &ts);
    f();
    clock_gettime(CLOCK_REALTIME, &ts2);
    printf("Used %d sec and %ld nsed.\n", ts2.tv_sec-ts.tv_sec, ts2.tv_nsec-ts.tv_nsec);
    return ts2.tv_nsec-ts.tv_nsec;
}
#define loop(n) for(size_t _ = 0; _ < n; ++_)
volatile int i = 0;
void f()
{
    loop(10)
        ++i;
}

void test_thread()
{
    auto th = new std::thread(f);
}

ThreadPool pool;

void test_threadpool()
{
    pool.addTask(f);
}

int main()
{
    long long sum1 = 0;
    loop(150)
    {
        loop(3)
        {
            sum1 += test(test_thread);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    long long sum2 = 0;
    loop(150)
    {
        loop(3)
        {
            sum2 += test(test_threadpool);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    printf("*****Sum1=%lld\n", sum1);
    printf("*****Sum2=%lld\n", sum2);
    printf("Test done. Here fuck SIGABRT:\n");
}