#ifndef CONNECT_POOL_H
#define CONNECT_POOL_H

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <pthread.h>

using namespace std;

template<class T>
class connect_pool
{
public:
    static connect_pool<T> *get_instance();
    bool init(vector<T*> connect_ptrs);
    int get_connect_index();
    T* get_connect(int index) const;
    bool return_connect_2_pool(int index);
    void remove_connect_from_pool(int index);
    bool replace_alive_connect(T* new_connect, int index);
private:
    connect_pool();
    ~connect_pool();
private:
    pthread_mutex_t *m_mutex;
    vector<int> m_used_index_vect;
    vector<T*> m_connect_vect;
};

#endif