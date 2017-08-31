#include "../lib/threadPool.hpp"
#include <iostream>
#include <unistd.h>
using namespace std;
int _runtime_debugLevel = 999;
void f()
{
    printf("Hello world!\n");
    printf("Hello world!\n");
    sleep(1);
    printf("done.\n");
}

void f2(int n)
{
    printf("Hello world%d!\n", n);
    sleep(1);
    printf("done%d.\n", n);
}

int main()
{
    ThreadPool pool;
    cout << "starting. poll.size=" << pool.size() << endl;
    pool.addTask(std::bind(f2, 111));
    pool.addTask(std::bind(f2, 222));
    pool.addTask(std::bind(f2, 333));
    pool.addTask(std::bind(f2, 444));
    pool.addTask(std::bind(f2, 555));
    pool.addTask(std::bind(f2, 666));
    pool.addTask(std::bind(f2, 111));
    pool.addTask(std::bind(f2, 222));
    pool.addTask(std::bind(f2, 333));
    pool.addTask(std::bind(f2, 444));
    pool.addTask(std::bind(f2, 555));
    pool.addTask(std::bind(f2, 666));
    pool.addTask(std::bind(f2, 111));
    pool.addTask(std::bind(f2, 222));
    pool.addTask(std::bind(f2, 333));
    pool.addTask(std::bind(f2, 444));
    pool.addTask(std::bind(f2, 555));
    pool.addTask(std::bind(f2, 666));
    cout << "1. poll.size=" << pool.size() << endl;
    //pool.addTask(std::bind(f2, 777));
    sleep(5);
    pool.addTask(std::bind(f2, 7777));
    pool.addTask(std::bind(f2, 337773));
    pool.addTask(std::bind(f2, 447774));
    pool.addTask(std::bind(f2, 557775));
    pool.addTask(std::bind(f2, 667776));
    //pool.addTask(std::bind(f2, 345));
    //pool.addTask(std::bind(f2, 1234));
    cout << "Done. poll.size=" << pool.size() << endl;
    pool.joinAll();
    cout << "joined. exiting..." << endl;
    return 0;
}