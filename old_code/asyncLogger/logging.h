#pragma once

#include "logStream.h"
#include <pthread.h>
#include <string.h>
#include <string>
#include <stdio.h>

class AsyncLogging;

//backend
class Logger
{
public:
    ~Logger();
    Logger(const char* filename, int line);
    LogStream& stream() { return impl_.stream_; }
private:
    class Impl
    {
    public:
        Impl(const char *fileName, int line);
        void formatTime();
        LogStream stream_;
        int line_;
        std::string basename_;
    };
    Impl impl_;
};

#define LOG Logger(__FILE__, __LINE__).stream()