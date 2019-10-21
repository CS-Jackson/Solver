#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <cstdio>

int main( int argc, char* argv[] )
{
    if( argc <= 2 )
    {
        printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi( argv[2] );

    struct sockaddr_in server_address;
    bzero( &server_address, sizeof( server_address ) );
    server_address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &server_address.sin_addr );
    server_address.sin_port = htons( port );

    int sockfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert( sockfd >= 0 );
    char sendline[4096], buffer[4096];
    if ( connect( sockfd, ( struct sockaddr* )&server_address, sizeof( server_address ) ) < 0 )
    {
        printf( "connection failed\n" );
    }
    struct stat sbuf;
    stat("httptest.txt", &sbuf);
    while(true){
        //fgets(sendline, 4096, stdin);
        int src_fd = open("httptest.txt", O_RDONLY, 0);
        char *src_addr = static_cast<char *>(mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
        close(src_fd);
        //if( send(sockfd, sendline, strlen(sendline), 0) < 0)
        if( send(sockfd, src_addr, sbuf.st_size, 0) < 0)
        {
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
            exit(0);
        }
        memset( buffer, '\0', 4096);
        if( (recv(sockfd, buffer, 4096, 0)) != 0 ){
            printf("Got msg from %d:\n", sockfd);
            printf("%s\n",buffer);
        }
        else {
            printf("Got nothing from %d\n", sockfd);
        }
    }

    close( sockfd );
    return 0;
}