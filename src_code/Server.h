#pragma once
#include "EventLoop.h"
#include "subProcess.h"
#include "EventLoopProcessPool.h"
#include <memory>


class Server
{
public:
    Server(EventLoop *loop, int ProcessNum, int port);
    ~Server() { }
    EventLoop* getLoop() const { return loop_; }
    void start();
    void handNewConn();
    void handTransConn(int fd_to_send);
    void handThisConn() { loop_->updatePoller(acceptChannel_); }

private:
    EventLoop *loop_;
    int ProcessNum_;
    std::unique_ptr<EventLoopProcesspool> eventLoopProcessPool_;
    bool started_;
    std::shared_ptr<Channel> acceptChannel_;
    int sub_process_fd;

    int port_;
    int listenFd_;
    static const int MAXFDS = 100000;
};