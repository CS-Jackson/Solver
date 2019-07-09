#include "Epoll.h"
#include "Util.h"
#include "threadpool.h"
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <queue>
#include <deque>

int TIMER_TIME_OUT = 500;
extern std::priority_queue<std::shared_ptr<mytimer>, std::deque<std::shared_ptr<mytimer>>, timerCmp> myTimerQueue;

epoll_event *Epoll::events;
std::unordered_map<int, std::shared_ptr<Solver>> Epoll::fd2req;
int Epoll::epoll_fd = 0;
const std::string Epoll::PATH = "/";

int Epoll::epoll_init(int maxevents, int listen_num)
{
    epoll_fd = epoll_create(listen_num + 1);
    if(epoll_fd == -1){
        return -1;
    }
    //
    events = new epoll_event[maxevents];
    return 0;
}

int Epoll::epoll_add(int fd, std::shared_ptr<Solver> solver, __uint32_t events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0){
        perror("epoll_add error");
        return -1;
    }
    fd2req[fd] = solver;
    return 0;
}

int Epoll::epoll_mod(int fd, std::shared_ptr<Solver> solver, __uint32_t events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0){
        perror("epoll_mod error");
        return -1;
    }
    fd2req[fd] = solver;
    return 0;
}

int Epoll::epoll_del(int fd, __uint32_t events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event) < 0){
        perror("epoll_del error");
        return -1;
    }
    auto fd_ite = fd2req.find(fd);
    if (fd_ite != fd2req.end()){
        fd2req.erase(fd_ite);
    }
    return 0;
}

void Epoll::my_epoll_wait(int listen_fd, int max_events, int timeout)
{
    int event_count = epoll_wait(epoll_fd, events, max_events, timeout);
    if(event_count < 0){
        if ( errno == EINTR)
        {
            printf("debug");
        }
        else{
            perror("epoll wait error");
        }
    }
    std::vector<std::shared_ptr<Solver>> solver_data = getEventsRequest(listen_fd, event_count, PATH);
    if (solver_data.size() > 0){
        for(auto &solver: solver_data){
            if(ThreadPool::threadpool_add(solver) < 0){
                perror("The Pool is full or shutdown.");
                break;
            }
        }
    }
}

#include <iostream>
#include <arpa/inet.h>
using namespace std;

void Epoll::acceptConnection(int listen_fd, int epoll_fd, const std::string path)
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = 0;
    int accept_fd = 0;
    while((accept_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
    {
        // cout << inet_addr(client_addr.sin_addr.s_addr) << endl; //这里感觉是写错了。
        // cout << inet_ntoa(client_addr.sin_addr) << endl;    //网络字节序和主机字节序的不同。
        // cout << ntohs(client_addr.sin_port) << endl;
        
        int ret = setnonblocking(accept_fd);
        if (ret < 0){
            perror("Set non block failed");
            return ;
        }

        std::shared_ptr<Solver> solver_info(new Solver(epoll_fd, accept_fd, path));

        //EPOLLIN, EPOLLET, EPOLLONESHOT;
        __uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
        Epoll::epoll_add(accept_fd, solver_info, _epo_event);
        //add timer;
        std::shared_ptr<mytimer> mtimer(new mytimer(solver_info, TIMER_TIME_OUT));
        solver_info->addTimer(mtimer);
        MutexLockGuard lock;
        myTimerQueue.push(mtimer);
    }
}

std::vector<std::shared_ptr<Solver>> Epoll::getEventsRequest(int listen_fd, int events_num, const std::string path)
{
    std::vector<std::shared_ptr<Solver>> solver_data;
    for(int i = 0; i < events_num; ++i)
    {
        // get fd
        int fd = events[i].data.fd;
        if(fd == listen_fd)
        {
            acceptConnection(listen_fd, epoll_fd, path);
        }
        else if ( fd < 3)
        {
            break;
        }
        else 
        {
            //error events
            if ( (events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))){
                auto fd_ite = fd2req.find(fd);
                if(fd_ite != fd2req.end())
                {
                    fd2req.erase(fd_ite);
                }
                continue;
            }
            //seperate the timer from solver before push into vector and the pool.
            std::shared_ptr<Solver> cur_req(fd2req[fd]);
            // printf("cur_rea.use_count before sep: %ld\n", cur_req.use_count());
            cur_req->seperateTimer();
            // printf("cur_rea.use_count after sep: %ld\n", cur_req.use_count());
            solver_data.push_back(cur_req);
            auto fd_ite = fd2req.find(fd);
            if(fd_ite != fd2req.end()){
                fd2req.erase(fd_ite);
            }
        }
    }
    return solver_data;
}


