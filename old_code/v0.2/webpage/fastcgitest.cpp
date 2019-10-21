#include <fcgi_stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unistd.h>


int main()
{
    int count = 0;

    while(FCGI_Accept() >= 0) {
        printf("Content-type: text/html\r\n");
        printf("\r\n");
        printf("<title>Fast CGI Hello!</title>");
        printf("<h1>Fast CGI Hello!</h1>");
        printf("Request number %d running on host <i>%s</i>\n", ++count, getenv("SERVER_NAME"));
        printf("QUERY: %S</br>", getenv("QUERY_sTRING"));
        printf("REMOTE_ADDR: %s</br>", getenv("REMOTE_ADDR"));
    }
    return 0;
}