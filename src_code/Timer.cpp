#include "Timer.h"
#include "Epoll.h"
#include <unordered_map>
#include <string>
#include <sys/time.h>
#include <deque>
#include <queue>

mytimer::mytimer(SP_Solver _solver_data, int timeout): deleted(false), solver_data(_solver_data)
{
    // cout << "mytimer()" << endl;
    struct timeval now;
    gettimeofday(&now , NULL);
    //以毫秒计
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

mytimer::~mytimer()
{
    // cout << "~mytimer()" << endl;
    if (solver_data) {
        solver_data->handleClose();
    }
}

mytimer::mytimer(mytimer &tn) : solver_data(tn.solver_data) { }

void mytimer::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool mytimer::isvalid()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t temp = ((now.tv_sec * 1000) + (now.tv_usec / 1000));
    if ( temp < expired_time)
    {
        return true;
    }
    else {
        this->setDeleted();
        return false;
    }
}

void mytimer::clearReq()
{
    solver_data.reset();
    this->setDeleted();
}

void mytimer::setDeleted()
{
    deleted = true;
}

bool mytimer::isDeleted() const
{
    return deleted;
}

size_t mytimer::getExpTime() const
{
    return expired_time;
}

HeapTimer::HeapTimer()
{

}
HeapTimer::~HeapTimer()
{

}

void HeapTimer::addTimer(SP_Solver solver_data, int timeout)
{
    SP_Timer new_node(new mytimer(solver_data, timeout));
    TimerQueue.push(new_node);  
    solver_data->linkTimer(new_node);
}


//将定时器的处理整合到一个类中，再把这个
void HeapTimer::handle_expired_event()
{
    // MutexLockGuard locker(lock);
    while(!TimerQueue.empty())
    {
        SP_Timer ptimer_now = TimerQueue.top();
        if(ptimer_now->isDeleted())
        {
            TimerQueue.pop();
        }
        else if(ptimer_now->isvalid() == false)
        {
            TimerQueue.pop();
        }
        else{
            break;
        }
    }
}
