#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>



// RAII锁
class MutexLockGuard
{
public:
    explicit MutexLockGuard();
    ~MutexLockGuard();
private:
    static pthread_mutex_t lock;
private:
    MutexLockGuard(const MutexLockGuard&);
    MutexLockGuard& operator=(const MutexLockGuard);
};

#endif
