#include "timer.h"

mytimer::mytimer(shared_ptr<Solver> _request_data, int timeout): deleted(false), solver_data(_request_data)
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
    if (solver_data)
    {
        Epoll::epoll_del(solver_data->getFd(), EPOLLIN | EPOLLET | EPOLLONESHOT);
    }
}

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

bool timerCmp::operator()(shared_ptr<mytimer> &a, shared_ptr<mytimer> &b) const
{
    return a->getExpTime() > b->getExpTime();
}
