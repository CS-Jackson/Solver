#pragma once
#include "httpparse.h"
#include "subProcess.h"
#include "Timer.h"
#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include <memory>

class Epoll
{
    //typedef std::shared_ptr<Channel> SP_Channel;
public:
    Epoll();
    ~Epoll();
    void epoll_add(SP_Channel request, int timeout);
    void epoll_mod(SP_Channel request, int timeout);
    void epoll_del(SP_Channel request);
    std::vector<SP_Channel> poll();
    std::vector<SP_Channel> getEventsRequest(int events_num);
    void add_timer(SP_Channel request_data, int timeout);
    int getEpollFd()
    {
        return epollFd_;
    }
    void handleExpired();
private:
    static const int MAXFDS = 100000;
    int epollFd_;
    std::vector<epoll_event> events_;
    SP_Channel fd2chan_[MAXFDS];
    std::shared_ptr<Solver> fd2http_[MAXFDS];
    HeapTimer timerManager_;
};