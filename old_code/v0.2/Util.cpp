#include "Util.h"
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>
#include <cassert>
#include "signal.h"

int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

void SetCloseOnExec(int sockfd){
    int flags=fcntl(sockfd,F_GETFD,0);
    flags|=FD_CLOEXEC;
    fcntl(sockfd,F_SETFL,flags);
}

void addfd( int epollfd, int fd )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}
void addfd( int epollfd, int fd, bool one_shot )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if( one_shot )
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}

void removefd( int epollfd, int fd )
{
    epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0 );
    close( fd );
}

void modfd( int epollfd, int fd, int ev )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl( epollfd, EPOLL_CTL_MOD, fd, &event );
}

// void addsig( int sig, void( handler )(int), bool restart = true )
// {
//     struct sigaction sa;
//     memset( &sa, '\0', sizeof( sa ) );
//     sa.sa_handler = handler;
//     if( restart )
//     {
//         sa.sa_flags |= SA_RESTART;
//     }
//     sigfillset( &sa.sa_mask );
//     assert( sigaction( sig, &sa, NULL ) != -1 );
// }

void handle_for_pipe(bool restart = true)
{
    
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    if( restart )
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset( &sa.sa_mask );
    // if(sigaction(SIGPIPE, &sa, NULL))
    //     return;
    assert( sigaction( SIGPIPE, &sa, NULL ) != -1 );
}