#pragma once
#ifndef UTIL_H
#define UTIL_H
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>
#include <cassert>
#include "signal.h"

int setnonblocking( int fd );
void SetCloseOnExec(int sockfd);
void handle_for_sigpipe();


#endif