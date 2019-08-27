#include "ProcessPool.h"

class cgi_conn
{
public:
    cgi_conn() { }
    ~cgi_conn() { }

    void init(int epollfd, int sockfd, const sockaddr_in& client_addr);
    void process();


private:
    static const int BUFFER_SIZE = 1024;
    static int m_epollfd;
    int m_sockfd;
    sockaddr_in m_address;
    char m_buf[BUFFER_SIZE];
    int m_read_idx;
};

