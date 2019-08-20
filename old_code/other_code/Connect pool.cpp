#include "Connect pool.hpp"

template<class T>
connect_pool<T>::connect_pool()
{
    m_mutex = new pthread_mutex_t;
    pthread_mutex_init(m_mutex, NULL);
    srand(time(NULL));
}

template<class T>
connect_pool<T>::~connect_pool()
{
    if(NULL != m_mutex) {
        delete m_mutex;
        m_mutex = NULL;
    }
}

template<class T>
connect_pool<T> * connect_pool<T>::get_instance()
{
    static connect<T> s_instance;
    return &s_instance;
}

template<class T>
bool connect_pool<T>::init(vector<T*> connect_ptrs)
{
    if(connect_ptrs.empty()) {
        return false;
    }
    pthread_mutex_lock(m_mutex);
    for(size_t i = 0; i < connect_ptrs.size(); i++) {
        m_used_index_vect.push_back(0);
        m_connect_vect.push_back(connect_ptrs[i]);
    }
    pthread_mutex_unlock(m_mutex);

    return true;
}

template<class T>
int connect_pool<T>::get_connect_index()
{
    int index = -1;
    int rand_index = 0;
    
    pthread_mutex_lock(m_mutex);

    if( 0 != m_used_index_vect.size()) {
        rand_index = rand() % m_used_index_vect.size();
    }
    for(int j = rand_index; j < m_used_index_vect.size(); j++) {
        if(0 == m_used_index_vect[j]) {
            m_used_index_vect[j] = 1;
            index = j;
            break;
        }
    }

    if(index == -1) {
        for(int i = 0; i < rand_index; i++) {
            if(0 == m_used_index_vect[i]) {
                m_used_index_vect[i] = 1;
                index = i;
                break;
            }
        }
    }
    pthread_mutex_unlock(m_mutex);
    return index;
}

template<class T>
T* connect_pool<T>::get_connect(int index) const 
{
    pthread_mutex_lock(m_mutex);
    if(index >= 0 && index < m_connect_vect.size()) {
        T* p = m_connect_vect[index];
        pthread_mutex_unlock(m_mutex);
        return p;
    }
    return nullptr;
}

template<class T>
bool connect_pool<T>::return_connect_2_pool(int index)
{
    if(index < 0) return false;

    pthread_mutex_lock(m_mutex);
    if(index < m_used_index_vect.size()) {
        m_used_index_vect[index] = 0;
        pthread_mutex_unlock(m_mutex);
        return true;
    }
    pthread_mutex_unlock(m_mutex);

    return false;
}

template<class T>
void connect_pool<T>::remove_connect_from_pool(int index)
{
    pthread_mutex_lock(m_mutex);
    if(index >= 0 && index <m_used_index_vect.size()) {
        m_used_index_vect[index] = 1;
    }
    pthread_mutex_unlock(m_mutex);
}

template<class T>
bool connect_pool<T>::replace_alive_connect(T* new_connect, int index)
{
    bool ret = false;
    pthread_mutex_lock(m_mutex);
    if(index >= 0 && index < m_used_index_vect.size()) {
        m_used_index_vect[index] = 0;
        m_connect_vect[index] = new_connect;
        ret =true;
    }
    pthread_mutex_unlock(m_mutex);
    return ret;
}





