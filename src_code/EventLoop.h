#pragma once
#include "Epoll.h"
#include "subProcess.h"
#include "Util.h"
#include <unistd.h>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>


class EventLoop
{
typedef std::function<void()> Functor;
private:
    bool looping_;
    std::shared_ptr<Epoll> poller_;
    int wakeupFd_;

    bool quit_;
    bool eventHandling_;
    
    std::vector<Functor> pendingFunctors_;
    bool callingPendingFunctors_;
    const pid_t process_Id;
    std::shared_ptr<Channel> pwakeupChannel_;

    void wakeup();
    void handleRead();
    void doPendingFunctors();
    void handleConn();
    static const int CONTROL_LEN = CMSG_LEN(sizeof(int));

public:
    EventLoop(int fd = -1);
    ~EventLoop();

    void loop();
    void quit();
    void RunInLoop(Functor&& cb);
    void QueueInLoop(Functor&& cb);
    bool isInLoopThread() const { return process_Id == getpid(); } 
    void set_pid();

    void assertInLoopProcess();
    void shutdown(std::shared_ptr<Channel> channel);
    void removeFromPoller(std::shared_ptr<Channel> channel)
    {
        poller_->epoll_del(channel);
    }
    void updatePoller(std::shared_ptr<Channel> channel, int timeout = 0)
    {
        poller_->epoll_mod(channel, timeout);
    }
    void addToPoller(std::shared_ptr<Channel> channel, int timeout = 0)
    {
        poller_->epoll_add(channel, timeout);
    }
};