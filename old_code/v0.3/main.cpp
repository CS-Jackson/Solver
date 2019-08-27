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
#include <iostream>
#include "threadpool.h"
#include "Epoll.h"
#include "Util.h"
#include "http_conn.h"
#include "Socket.h"
// #include "Mysql.h"

static const int MAX_EVENT_NUMBER = 5000;
static const int LISTENQ = 1024;
const int THREADPOOL_THREAD_NUM = 4;
const int QUEUE_SIZE = 65535;
const int PORT = 12345;
const int TIMER_TIME_OUT = 500;

// const std::string PATH = "/";

//可以将timer_queue封装到Epoll里面，因为它负责检测新事件，顺便对计时器做出改动。
// extern std::priority_queue<std::shared_ptr<mytimer>, std::deque<std::shared_ptr<mytimer>>, timerCmp> myTimerQueue;
int socket_bind_listen(int port)
{
    // 检查port值，取正确区间范围
    if (port < 1024 || port > 65535)
        return -1;

    // 创建socket(IPv4 + TCP)，返回监听描述符
    int listen_fd = 0;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;

    // 消除bind时"Address already in use"错误
    int optval = 1;
    if(setsockopt(listen_fd, SOL_SOCKET,  SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        return -1;

    // 设置服务器IP和Port，和监听描述副绑定
    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);
    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        return -1;

    // 开始监听，最大等待队列长为LISTENQ
    if(listen(listen_fd, LISTENQ) == -1)
        return -1;

    // 无效监听描述符
    if(listen_fd == -1)
    {
        close(listen_fd);
        return -1;
    }
    return listen_fd;
}

// void handle_expired_event()
// {
//     MutexLockGuard lock;    //构造类的同时上锁。
//     while(!myTimerQueue.empty())
//     {
//         std::shared_ptr<mytimer> ptimer_now = myTimerQueue.top();
//         if(ptimer_now->isDeleted())
//         {
//             // cout << "Delete the timer because isDeleed" << endl;
//             myTimerQueue.pop();//智能指针自动销毁
//         }
//         else if (ptimer_now->isvalid() == false)
//         {
//             cout << "Delete the timer because isvalid" << endl;
//             myTimerQueue.pop();
//         }
//         else{
//             break;
//         }
//     }
// }

int main( int argc, char* argv[] )
{
    // if( argc <= 2 )
    // {
    //     printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
    //     return 1;
    // }
    // const char* ip = argv[1];
    // int port = atoi( argv[2] );
    #ifndef _PTHREADS
        printf("_Pthreads is not defined!\n");
    #endif

    // MyDB db;
    // db.initDB("localhost", "root", "csj08220618", "test");
    // db.exeSQL("select * from userinfo");

    handle_for_sigpipe();

    if(Epoll::epoll_init(MAX_EVENT_NUMBER, LISTENQ) < 0)
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

    // int listenfd = socket_bind_listen(PORT);
    // if (listenfd < 0) 
    // {
    //     perror("socket bind failed");
    //     return 1;
    // }
    // if (setnonblocking(listenfd) < 0)
    // {
    //     perror("set socket non block failed");
    //     return 1;
    // }

    std::shared_ptr<Solver> solver(new Solver());
    solver->setFd(listenfd);

    if (Epoll::epoll_add(listenfd, solver, EPOLLIN | EPOLLET) < 0){
        perror("epoll add error.");
        return 1;
    }
    std::cout << "Begin loop" << std::endl;
    while( 1 )
    {
        Epoll::my_epoll_wait(listenfd, MAX_EVENT_NUMBER, -1);
        // handle_expired_event();
    }

    // close( epollfd );
    // close( listenfd );
    
    return 0;
}
