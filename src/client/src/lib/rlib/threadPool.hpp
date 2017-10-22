#ifndef _SNAKESOCKSCLI_THREADPOOL_HPP
#define _SNAKESOCKSCLI_THREADPOOL_HPP 1
/*
 * This thread pool is specially designed for snakesocks.
 * Its time complexity is O(n^2) if steadily inserted heavy work,
 *      in which case raw pthread_create performs better.
 *
 * by R.K., GNU Licensed.
 */

#include <functional>
#include <list>
#include <boost/core/noncopyable.hpp>
#include <thread>
#include <mutex>

class ThreadPool : private boost::noncopyable
{ //Note: The threadPool itself is not thread safe.
    class worker;
public:
    ThreadPool(size_t usualSize = 4) : usualSize(usualSize) {}
    ~ThreadPool() {
        for(auto pwk : workers)
            delete pwk;
    }
    void addTask(std::function<void(void)> &&f)
    {
        std::function<void(void)> decayed_f(f);
        addTask(decayed_f);
    }
    void addTask(std::function<void(void)> &f)
    {
        for(auto pwk : workers)
        {
            if(!pwk->isWaiting)
                continue;
            pwk->go(f);
            return;
        }
        //No spare worker
        newWorker()->go(f);
    }
    void joinAll()
    {
        for(auto pwk : workers)
            pwk->lazy_stop();
        for(auto pwk : workers) {
            pwk->join();
            delete pwk;
        }
        workers.clear();
        currSize = 0;
    }
    size_t size()
    {
        return currSize;
    }
private:
    worker *newWorker()
    {
        try_shrink();
        worker *pwk = new worker();
        while(!pwk->initSymbolInited);
        workers.push_back(pwk);
        ++currSize;
        pwk->initSymbol.lock(); //Wait for child inited.
        return pwk;
    }
    void try_shrink()
    {
        if(currSize <= usualSize * 3) return;
        for(auto pwk : workers)
        {
            if(!pwk->isWaiting) continue;
            if(currSize <= usualSize) return;
            pwk->join();
            delete pwk;
            workers.remove(pwk);
            --currSize;
        }
    }
    size_t usualSize;
    size_t currSize = 0;
    std::list<worker *> workers;

    class worker
    {
    private:
        void workerFunc()
        {
            initSymbol.try_lock();
            initSymbolInited = true;
            bool firstLoop = true;
            while(continueLoop)
            {
                workingSymbol.lock();
                {
                    if(firstLoop)
                    {
                        firstLoop = false;
                        initSymbol.unlock();
                    }
                    isWaiting = true;
                    std::lock_guard<std::mutex> lg(workingSymbol); //Waiting here usually.
                    isWaiting = false;
                    task();
                }
            }
        }
    public:
        worker() : isWaiting(false), continueLoop(true), initSymbolInited(false), m_thread(std::bind(&worker::workerFunc, this)) {}
        worker(worker &&ano) = delete;
        void go(std::function<void(void)> &&f)
        {
            task = f;
            if(workingSymbol.try_lock()) printf("Here exciting!!!\n");
            workingSymbol.unlock();
        }
        void go(std::function<void(void)> &f)
        {
            task = f;
            if(workingSymbol.try_lock()) printf("Here exciting!!!\n");
            workingSymbol.unlock();
            while(isWaiting);
        }
        void join()
        {
            if(continueLoop)
                continueLoop = false;
            if(isWaiting)
                go([](){});
            m_thread.join();
        }
        void lazy_stop()
        {
            continueLoop = false;
        }
        volatile bool isWaiting;
        std::mutex initSymbol;
        volatile bool initSymbolInited = false;
    private:
        bool continueLoop;
        std::mutex workingSymbol; //Locked on working or waiting. Unlock it to notify worker.
        std::thread m_thread;
        std::function<void(void)> task;
    };
};

#endif //_SNAKESOCKSCLI_THREADPOOL_HPP
