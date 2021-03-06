//
// Latest edit by TeeKee on 2017/7/23.
//

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include "rio.h"

ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    ssize_t readSum = 0;        //在网络编程中，有可能会因为没有数据所以报错的情况，errno == EAGAIN，这时候要及时返回读了多少而不是报错。
    char *bufp = (char *)usrbuf;
    while(nleft > 0){
        if((nread = read(fd, bufp, nleft)) < 0){
            if(errno == EINTR)
            {
                nread = 0;
            }
            else if( errno == EAGAIN){
                return readSum;
            }
            else{
                return -1;
            }
        }
        else if(nread == 0)
            break;
        readSum += nread;
        nleft -= nread;
        bufp += nread;
    }
    return readSum;
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t writeSum = 0;
    ssize_t nwritten;
    char *bufp = (char *)usrbuf;

    while(nleft > 0){
        if((nwritten = write(fd, bufp, nleft)) <= 0){
            if (errno == EINTR || errno == EAGAIN)
            {
                nwritten = 0;
                continue ;
            }
            else{
                return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    size_t cnt;
    while(rp->rio_cnt <= 0){
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0){
            if(errno == EAGAIN){
                return -EAGAIN;
            }
            if(errno != EINTR){
                return -1;
            }
        }
        else if(rp->rio_cnt == 0)
            return 0;
        else
            rp->rio_bufptr = rp->rio_buf;
    }
    cnt = n;
    if(rp->rio_cnt < (ssize_t)n)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

const int MAX_BUFF = 4096;
ssize_t rio_readn(int fd, std::string &inBuffer)
{
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while(true)
    {
        char buff[MAX_BUFF];
        if ( (nread = read(fd, buff, MAX_BUFF)) < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            else if ( errno == EAGAIN)
            {
                return readSum;
            }
            else {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0){
            break;
        }
        readSum += nread;
        // std::cout << "Before reading Buffer's size: " << inBuffer.size() << std::endl;
        inBuffer += std::string(buff, buff+nread);
        // std::cout << "After reading Buffer's size: " << inBuffer.size() << std::endl;
    }
    return readSum;
}

ssize_t rio_writen(int fd, std::string &sbuff)
{
    size_t nleft = sbuff.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    const char *ptr = sbuff.c_str();
    while(nleft > 0)
    {
        if( (nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if(nwritten < 0){
                if(errno == EINTR){
                    nwritten = 0;
                    continue;
                }
                else if (errno == EAGAIN){
                    break;
                }
                else
                {
                    return -1;
                }
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    if (writeSum == sbuff.size()){
        sbuff.clear();
    }
    else {
        sbuff = sbuff.substr(writeSum);
    }
    return writeSum;
}


void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char *)usrbuf;
    while (nleft > 0){
        if((nread = rio_read(rp, bufp, nleft)) < 0){
            if(errno == EINTR)
                nread = 0;
            else
                return -1;
        }
        else if(nread == 0)
            break;
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    size_t n;
    ssize_t rc;
    char c, *bufp = (char *)usrbuf;
    for(n = 1; n < maxlen; n++){
        if((rc = rio_read(rp, &c, 1)) == 1){
            *bufp++ = c;
            if(c == '\n')
            break;
        }
        else if(rc == 0){
            if (n == 1){
                return 0;
            }
            else
                break;
        }
        else if(rc == -EAGAIN){
            return rc;
        }
        else{
            return -1;
        }
    }
    *bufp = 0;
    return n;
}
