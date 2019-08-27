#pragma once
#pragma once
#include "http_conn.h"
#include "timer.h"
#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include <memory>


class Epoll
{
public:
    typedef std::shared_ptr<Solver> SP_Solver;
private:
    static const int MAX_FDs = 1000;
    static epoll_event *events;
    // static std::unordered_map<int, std::shared_ptr<Solver>> fd2req;
    static SP_Solver fd2req[MAX_FDs];
    static int epoll_fd;
    static const std::string PATH;

    static HeapTimer heaptimer;

public:
    static int epoll_init(int maxevents, int listen_num);
    static int epoll_add(int fd, SP_Solver request, __uint32_t events);
    static int epoll_mod(int fd, SP_Solver request, __uint32_t events);
    static int epoll_del(int fd, __uint32_t events = (EPOLLIN | EPOLLET | EPOLLONESHOT));
    static void my_epoll_wait(int listen_fd, int max_events, int timeout);
    static void acceptConnection(int listen_fd, int epoll_fd, const std::string path);
    static std::vector<SP_Solver> getEventsRequest(int listen_fd, int events_num, const std::string path);

    static void add_timer(SP_Solver solver, int timeout);
};