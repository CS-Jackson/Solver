#include <vector>
#include <string>
#include <queue>
#include <memory>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "threadpool.h"
#include "Epoll.h"
#include "Util.h"
#include "http_conn.h"
#include "locker.h"

static const int MAX_FD = 1024;
static const int MAX_EVENT_NUMBER = 5000;
const int THREADPOOL_THREAD_NUM = 4;
const int QUEUE_SIZE = 65535;
const int PORT = 12345;

// const int ASK_STATIC_FILE = 1;
// const int ASK_IMAGE_STITCH = 2;

const std::string PATH = "/";

extern std::priority_queue<std::shared_ptr<mytimer>, std::deque<std::shared_ptr<mytimer>>, timerCmp> myTimerQueue;

void handle_expired_event()
{
    MutexLockGuard lock;    //构造类的同时上锁。
    while(!myTimerQueue.empty())
    {
        std::shared_ptr<mytimer> ptimer_now = myTimerQueue.top();
        if(ptimer_now->isDeleted())
        {
            myTimerQueue.pop();//智能指针自动销毁
        }
        else if (ptimer_now->isvalid() == false)
        {
            myTimerQueue.pop();
        }
        else{
            break;
        }
    }
}

int main( int argc, char* argv[] )
{
    if( argc <= 2 )
    {
        printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi( argv[2] );

    handle_for_sigpipe();

    if(Epoll::epoll_init(MAX_EVENT_NUMBER, MAX_FD) < 0)
    {
        perror("epoll init failed");
        return 1;
    }
    if(ThreadPool::threadpool_create(THREADPOOL_THREAD_NUM, QUEUE_SIZE) < 0)
    {
        printf("Threadpool create failed\n");
        return 1;
    }

    // Socket listensocket(port, ip);
    Socket listensocket(PORT);
    listensocket.bind_fd();
    int listenfd = listensocket.listen_fd();

    std::shared_ptr<Solver> solver(new Solver());
    solver->setFd(listenfd);

    while( 1 )
    {
        Epoll::my_epoll_wait(listenfd, MAX_EVENT_NUMBER, -1);
        handle_expired_event();
    }

    // close( epollfd );
    // close( listenfd );
    
    return 0;
}
