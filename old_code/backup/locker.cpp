#include "locker.h"

sem::sem()
    {
        if( sem_init( &m_sem, 0, 0 ) != 0 )
        {
            throw std::exception();
        }
    }
sem::~sem()
    {
        sem_destroy( &m_sem );
    }
bool sem::wait()
    {
        return sem_wait( &m_sem ) == 0;
    }
bool sem::post()
    {
        return sem_post( &m_sem ) == 0;
    }


locker::locker()
    {
        if( pthread_mutex_init( &m_mutex, NULL ) != 0 )
        {
            throw std::exception();
        }
    }
locker::~locker()
    {
        pthread_mutex_destroy( &m_mutex );
    }
bool locker::lock()
    {
        return pthread_mutex_lock( &m_mutex ) == 0;
    }
bool locker::unlock()
    {
        return pthread_mutex_unlock( &m_mutex ) == 0;
    }
