
#ifndef BB_STD_ASYNC_H
#define BB_STD_ASYNC_H

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "process.hpp"
#include <iostream>

namespace Async
{


    struct Semaphore
    {
        int count;
        std::mutex mutex;
        std::condition_variable cond_var;

        Semaphore() : count(0)
        {
        }

        Semaphore(int count) : count(count)
        {
        }

        void wait()
        {
            std::unique_lock<std::mutex> lock(mutex);
            while (!count)
                cond_var.wait(lock);
            --count;
        }

        void signal()
        {
            std::unique_lock<std::mutex> lock(mutex);
            ++count;
            cond_var.notify_one();
        }
    };

    struct Event
    {
        virtual void dispatch() = 0;
    };

    void setPostEventFilter(void (*filter)(Event *));

} // namespace Async


class ProcessOutputEvent : public Async::Event
{
public:
    ProcessOutputEvent(const std::string& output) : output_(output)
    {
        std::cout << "ProcessOutputEvent" << std::endl;
    }

    virtual void dispatch() override
    {
        std::cout << "ProcessOutputEvent::dispatch" << std::endl;
    }

private:
    std::string output_;
    
};



#endif