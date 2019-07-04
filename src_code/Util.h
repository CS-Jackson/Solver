#pragma once
#ifndef UTIL_H
#define UTIL_H

#define FD_LIMIT 65535
#define TIMESLOT 5

static int pipefd[2];
static int epollfd = 0;

int setnonblocking( int fd );
void SetCloseOnExec(int sockfd);

void addfd( int epollfd, int fd );
void addfd( int epollfd, int fd, bool oneshot);
void removefd( int epollfd, int fd );
void modfd( int epollfd, int fd, int ev );

void handle_for_pipe();

#endif