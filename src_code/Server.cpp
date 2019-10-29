#include "Server.h"
#include "Util.h"
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

Server::Server(EventLoop *loop, int ProcessNum, int port)
: loop_(loop), ProcessNum_(ProcessNum), 
  eventLoopProcessPool_(new EventLoopProcesspool(loop_, ProcessNum)),
  started_(false), acceptChannel_(new Channel(loop_)), sub_process_fd(-1), port_(port), 
  listenFd_(socket_bind_listen(port_))
{
    acceptChannel_->setFd(listenFd_);
    // eventLoopProcessPool_->setListenfd(listenFd_);
    handle_for_sigpipe();
    if(setSocketNonBlocking(listenFd_) < 0) {
        perror("Set socket non block failed");
        abort();
    }
}

void Server::start()
{
    eventLoopProcessPool_->run();
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
    acceptChannel_->setReadHandler(std::bind(&Server::handNewConn, this));
    acceptChannel_->setConnHandler(std::bind(&Server::handThisConn, this));
    
    loop_->addToPoller(acceptChannel_, 0);
    started_ = true;
}

void Server::handNewConn()
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    while( (accept_fd = accept(listenFd_, (struct sockaddr*)&client_addr, &client_addr_len)) > 0) {

        cout << "new connection" << endl;
        // cout << inet_ntoa(client_addr.sin_addr) << endl;
        // cout << ntohs(client_addr.sin_port) << endl;
        if(accept_fd >= MAXFDS) {
            close(accept_fd);
            continue;
        }
        if(setSocketNonBlocking(accept_fd) < 0) {
            perror("Set non block failed!");
            return;
        }
        setSocketNodelay(accept_fd);
        //setSocketNoLinger(accept_fd);
        sub_process_fd = eventLoopProcessPool_->getNextprocess();
        handTransConn(accept_fd);
    } 
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}

static const int CONTROL_LEN = CMSG_LEN(sizeof(int));

void Server::handTransConn(int fd_to_send)
{
    if(sub_process_fd < 0) {
        std::cout << "Wrong fd to trans" << std::endl;
        abort();
    }
    cout << "Sending fd " << endl;
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
    cm.cmsg_len = CONTROL_LEN;
    cm.cmsg_level = SOL_SOCKET;
    cm.cmsg_type = SCM_RIGHTS;
    *(int *)CMSG_DATA(&cm) = fd_to_send;
    msg.msg_control = &cm;
    msg.msg_controllen = CONTROL_LEN;
    sendmsg(sub_process_fd, &msg, 0);
    cout << "Done send" << endl;
}
