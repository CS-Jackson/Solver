#include "logging.h"

#include "perthread.h"
#include <assert.h>

#include "asyncLogging.h"

#include <iostream>
using namespace std;

static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging *AsyncLogger_;

void once_init()
{
    AsyncLogger_ = new AsyncLogging(std::string("/SolverServer.log"));
    AsyncLogger_->start();
}

void output(const char* msg, int len)
{
    pthread_once(&once_control_, once_init);
    AsyncLogger_->append(msg, len);
}

Logger::Impl::Impl(const char *filename, int line)
    : stream_(), basename_(filename), line_(line)
{
    //formatTime();
    //CurrentThread::tid();
    //stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
}

void Logger::Impl::formatTime()
{ }

Logger::Logger(const char *filename, int line) : impl_(filename, line) { }

Logger::~Logger()
{
    impl_.stream_ << " - " << impl_.basename_ << ':' << impl_.line_ << '\n';
    const LogStream::Buffer& buf(stream().buffer());
    output(buf.data(), buf.length());
}