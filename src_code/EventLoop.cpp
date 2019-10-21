#include "EventLoop.h"
#include "Util.h"
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <iostream>
using namespace std;

//Used to IPC though threads;
int createEventfd()
{
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        // LOG << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop(int a)
:   looping_(false),
    poller_(new Epoll()),
    wakeupFd_(a),
    quit_(false), process_Id(getpid()),
    eventHandling_(false),
    callingPendingFunctors_(false),
    pwakeupChannel_(new Channel(this))
{
    if(wakeupFd_ > 0) {
        pwakeupChannel_->setFd(wakeupFd_);
        //pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
        pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
        pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this));
        pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this));
        poller_->epoll_add(pwakeupChannel_, 0);
    }
}

void EventLoop::handleConn()
{
    //poller_->epoll_mod(wakeupFd_, pwakeupChannel_, (EPOLLIN | EPOLLET | EPOLLONESHOT), 0);
    updatePoller(pwakeupChannel_, 0);
}


EventLoop::~EventLoop()
{
    //wakeupChannel_->disableAll();
    //wakeupChannel_->remove();
    if(wakeupFd_ > 0)
        close(wakeupFd_);
}

// void EventLoop::wakeup()
// {
//     uint64_t one = 1;
//     ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
//     if (n != sizeof one)
//     {
//         //LOG<< "EventLoop::wakeup() writes " << n << " bytes instead of 8";
//         cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
//     }
// }

void EventLoop::handleRead()
{
    struct iovec iov[1];
    struct msghdr msg;
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    cmsghdr cm;
    msg.msg_control = &cm;
    msg.msg_controllen = CONTROL_LEN;

    recvmsg(wakeupFd_, &msg, 0);
    int fd_to_read = *(int *)CMSG_DATA(&cm);
    std::shared_ptr<Solver> req_info (new Solver(this, fd_to_read));
    req_info->getChannel()->setHolder(req_info);
    QueueInLoop(std::bind(&Solver::newEvent, req_info));
    //pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::RunInLoop(Functor&& cb)
{
    cb();
    // if (isInLoopThread())
    //     cb();
    // else
    //     QueueInLoop(std::move(cb));
}

void EventLoop::QueueInLoop(Functor&& cb)
{
    pendingFunctors_.emplace_back(std::move(cb));
    // {
    //     MutexLockGuard lock(mutex_);
    //     pendingFunctors_.emplace_back(std::move(cb));
    // }

    // if (!isInLoopThread() || callingPendingFunctors_)
    //     wakeup();
}

void EventLoop::loop()
{
    assert(!looping_);
    
    looping_ = true;
    quit_ = false;
    //LOG_TRACE << "EventLoop " << this << " start looping";
    std::vector<SP_Channel> ret;
    while (!quit_)
    {
        //cout << "doing" << endl;
        ret.clear();
        ret = poller_->poll();
        eventHandling_ = true;
        for (auto &it : ret)
            it->handleEvents();
        eventHandling_ = false;
        doPendingFunctors();
        poller_->handleExpired();
    }
    looping_ = false;
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    functors.swap(pendingFunctors_);
    // {
    //     MutexLockGuard lock(mutex_);
    //     functors.swap(pendingFunctors_);
    // }

    for (size_t i = 0; i < functors.size(); ++i)
        functors[i]();
    callingPendingFunctors_ = false;
}

// void EventLoop::quit()
// {
//     quit_ = true;
//     if (!isInLoopThread())
//     {
//         wakeup();
//     }
// }