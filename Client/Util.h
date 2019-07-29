#pragma once
#ifndef UTIL_H
#define UTIL_H
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>
#include <cassert>
#include "signal.h"
#include "rio.h"
#include<vector>
#include<sstream>
#include<opencv2/opencv.hpp>
#include<memory>
#include<string>
using std::string;
using std::shared_ptr;
using cv::Mat;

enum METHOD{MEDIANBLUR,SAVE,GET, POST};
void split(const std::string &s,char delim,std::vector<std::string>&elems);
std::vector<std::string> split(const std::string &s,char delim);
int ReadInt(int acceptfd);
string ReadString(int acceptfd);
Mat readMat(int acceptfd);
string image2string(const Mat& image);
string process2string(int method,const string&imagename,const Mat&image);

int setnonblocking( int fd );
void SetCloseOnExec(int sockfd);
void handle_for_sigpipe();


#endif