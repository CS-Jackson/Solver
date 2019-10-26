#pragma once
#include "subProcess.h"
#include "EventLoop.h"
#include <functional>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>


class EventLoopProcesspool
{
public:
    EventLoopProcesspool(EventLoop *loop, int ProcessNum) 
    : baseloop_(loop), ProcessNum_(ProcessNum), next_(0), m_stop(false), started_(false)
    {
        if(ProcessNum_ <= 0 || ProcessNum_ > MAX_PROCESS_NUMBER) {
            ProcessNum_ = MAX_PROCESS_NUMBER;
            abort();
        }
        
    }
    ~EventLoopProcesspool() {
        std::cout << "~EventLoopProcesspool()" << std::endl;
    }
    void run();
    //void setListenfd(int listen_fd) { listen_fd_ = listen_fd;}
    int getNextprocess();
    //void set_acceptfd_subprocess(int accept_fd);

private:
    void run_parent();
    void run_child(int idx, pid_t pid, int sockpair);

private:
    EventLoop* baseloop_;
    int ProcessNum_;
    //int listen_fd_;
    int next_;  //choose which sub_process to work
    bool m_stop;
    bool started_;
    int sig_pipe[2];
    SP_Channel sig_channel;
    static std::vector<std::unique_ptr<process>> processes;
    static const int MAX_PROCESS_NUMBER = 4;
    static const int USER_PER_PROCESS = 65536;
    static const int MAX_EVENT_NUMBER = 10000;

    static void sig_handler(int sig)//wait for children.
    {
        // int save_errno = errno;
        // int msg = sig;
        // send(sig_pipe[1], (char *)&msg, 1, 0);
        // errno = save_errno;
        pid_t   pid;
        int     stat;
      
        while((pid = waitpid(-1, &stat, WNOHANG)) > 0){
            printf("child %d terminated\n", pid);
            for(int i = 0; i < processes.size(); ++i) {//Warning: signed compared with unsigned.
                if(processes[i]->m_pid == pid) {
                    close(processes[i]->m_pipefd);
                    processes[i]->m_pid = -1;
                }
            }
        }
        
        return;
    }

    void addsig(int sig, void(*handler)(int), bool restart = true)
    {
        struct sigaction sa;
        memset(&sa, '\0', sizeof(sa));
        sa.sa_handler = handler;
        if(restart) {
            sa.sa_flags |= SA_RESTART;
        }
        sigfillset(&sa.sa_mask);
        assert(sigaction(sig, &sa, NULL) != -1);
    }
};

