#include "Util.h"

void SetCloseOnExec(int sockfd){
    int flags=fcntl(sockfd, F_GETFD, 0);
    flags|=FD_CLOEXEC;
    fcntl(sockfd, F_SETFL, flags);
}

void handle_for_sigpipe(){
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if(sigaction(SIGPIPE, &sa, NULL))
        return;
}

int setnonblocking( int fd )
{
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1)
        return -1;

    flag |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    return 0;
}