#pragma once

#include "countDownLatch.h"
#include "mutexlock.h"
#include "perthread.h"
#include "logStream.h"
#include "noncopyable.h"
#include <functional>
#include <string>
#include <vector>

//from frontend to backend
class AsyncLogging : noncopyable
{
public:
    AsyncLogging(const std::string basename, int flushInterval = 2);
    ~AsyncLogging() { if(running_) stop(); }

    void append(const char* logline, int len);

    void start() {
        running_ = true;
        thread_.start();
        latch_.wait();
    }

    void stop() {
        running_ = false;
        cond_.notify();
        thread_.join();
    }
    AsyncLogging(const AsyncLogging&) = delete;
    void operator=(const AsyncLogging&) = delete;

private:
    void threadFunc();
    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
    typedef std::shared_ptr<Buffer> BufferPtr;

    const int flushInterval_;
    bool running_;
    std::string basename_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    BufferPtr currentBuffer_;   //ready to recv from worker
    BufferPtr nextBuffer_;      //ready to write on harddrive
    BufferVector buffers_;
    CountDownLatch latch_;
};

