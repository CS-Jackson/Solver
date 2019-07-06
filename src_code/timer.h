#pragma once
#include <iostream>
#include <sys/time.h>
#include "http_conn.h"

using namespace std;

struct mytimer
{
    bool deleted;
    size_t expired_time;
    shared_ptr<Solver> solver_data;
    mytimer( shared_ptr<Solver> _solver_data, int timeout);
    ~mytimer();
    void update(int timeout);
    bool isvalid();
    void clearReq();
    void setDeleted();
    bool isDeleted() const;
    size_t getExpTime() const;
};

struct timerCmp
{
    bool operator()(std::shared_ptr<mytimer> &a, std::shared_ptr<mytimer> &b) const;
};



