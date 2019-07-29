#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>


void cleanup(void *arg)
{
    printf("cleanup: %s\n", (char *)arg);
}

void *thr_fn1(void *arg)
{
    printf("thread 1 start\n");
    pthread_cleanup_push(cleanup, (void*)("thread 1 first handler"));
    pthread_cleanup_push(cleanup, (void*)("thread 1 second handler"));
    printf("thread 1 push complete\n");
    if (arg)
    {
        return ((void*) -1);
    }
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    return ((void *)1);
}

void *thr_fn2(void *arg)
{
    printf("thread 2 start\n");
    pthread_cleanup_push(cleanup, (void*)("thread 2 first handler"));
    pthread_cleanup_push(cleanup, (void*)("thread 2 second handler"));
    printf("thread 2 push complete\n");
    if (arg)
    {
        return ((void*) -1);
    }
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    // return ((void *)1);
    pthread_exit((void *)2);
}
pthread_t ntid;
void printids(const char *s)
{
    pid_t pid;
    pthread_t tid;
    pid = getpid();
    tid = pthread_self();
    printf("%s pid %lu tid %lu (0x%lx)\n", s, (unsigned long)pid, (unsigned long)tid, (unsigned long)tid);
}

void *thr_fn(void *arg)
{
    printids("new thread: ");
    return ((void *)0);
}

void *thr_fn_exit(void *arg)
{
    printids("thread 1: ");
    printf("thread 1 returning\n");
    sleep(1);
    return ((void*) 1);
}
void *thr_fn_exit1(void *arg)
{
    printids("thread 2: ");
    printf("thread 2 exiting\n");
    sleep(1);
    pthread_exit( (void*) 2);
}

int main()
{
    int err;
	pthread_t tid1, tid2;
	void *tret;
    printids("Main thread: ");
	err = pthread_create(&tid1,NULL,thr_fn_exit, NULL);
	if(err != 0)
		printf("creat error:%s\n",strerror(err));
    
    err = pthread_create(&tid2, NULL, thr_fn_exit1, NULL);
    if (err != 0)
        printf("creat error:%s\n",strerror(err));
	err = pthread_join(tid1, &tret);
    printf("thread 1 exit code %ld\n", (long)tret);
    err = pthread_join(tid2, &tret);
	if(err != 0)
		printf("get end error:%s\n",strerror(err));
	printf("thread 2 exit code %ld\n",(long)tret);
	return 0;

    // int err;
    // err = pthread_create(&ntid, NULL, thr_fn, NULL);
    // if (err != 0)
    //     printf("Can't create thread");
    // printids("main thread: ");
    // sleep(1);
    // exit(0);

	// int err;
	// pthread_t tid1, tid2;
	// void *tret;
	// err = pthread_create(&tid1,NULL,thr_fn1, (void *)1);
	// if(err != 0)
	// 	printf("creat error:%s\n",strerror(err));
    // err = pthread_create(&tid2, NULL, thr_fn2, (void *)2);
    // if (err != 0)
    //     printf("creat error:%s\n",strerror(err));
    // err = pthread_join(tid1, &tret);
    // printf("thread 1 exit code %ld\n", (long)tret);
    // err = pthread_join(tid2, &tret);
	// if(err != 0)
	// 	printf("get end error:%s\n",strerror(err));
	// printf("thread 2 exit code %ld\n",(long)tret);
	// return 0;
}
