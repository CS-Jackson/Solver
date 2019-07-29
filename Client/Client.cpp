#include "Util.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <memory>
#include <string>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

int main() 
{
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(12345);
    if( connect(sockfd, (const sockaddr*)&server_addr, sizeof(server_addr) ) < 0) {
        printf( "connection failed\n" );
        close(sockfd);
        return 1;
    }
    Mat image;
    METHOD methodname = POST;
    string requeststring;
    image = imread("lena.jpg");
    requeststring = process2string((int)methodname, "lena.jpg", image);
    cout << requeststring << endl;
    rio_writen(sockfd, requeststring);
    close(sockfd);
    return 0;
}

