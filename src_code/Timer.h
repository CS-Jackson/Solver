#pragma once
#include "httpparse.h"
#include "base/nocopyable.hpp"
#include "base/locker.hpp"
#include <unistd.h>
#include <memory>
#include <queue>
#include <deque>

// #include <iostream>
// using namespace std;

class Solver;

class mytimer
{
    typedef std::shared_ptr<Solver> SP_Solver;
private:
    bool deleted;
    size_t expired_time;
    SP_Solver solver_data;
public:
    mytimer(SP_Solver _solver_data, int timeout);
    ~mytimer();
    mytimer(mytimer &tn);
    void update(int timeout);
    bool isvalid();
    void clearReq();
    void setDeleted();
    bool isDeleted() const;
    size_t getExpTime() const;
};

struct timerCmp
{
    bool operator()(std::shared_ptr<mytimer> &a, std::shared_ptr<mytimer> &b) const
    {
        return a->getExpTime() > b->getExpTime();
    }
};

class HeapTimer
{
    typedef std::shared_ptr<Solver> SP_Solver;
    typedef std::shared_ptr<mytimer> SP_Timer;
private:
    std::priority_queue<SP_Timer, std::deque<SP_Timer>, timerCmp> TimerQueue;
    //MutexLock lock; 

public:
    HeapTimer();
    ~HeapTimer();
    void addTimer(SP_Solver Solver, int timeout);
    void handle_expired_event();
};

