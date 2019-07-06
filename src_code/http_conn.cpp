#include "http_conn.h"

#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdlib>
#include <unordered_map>
#include <queue>
#include <fcntl.h>

#include "Util.h"
#include "Epoll.h"
#include "locker.h"
#include "timer.h"

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
using namespace cv;

#include <iostream>
using namespace std;

pthread_mutex_t MimeType::lock = PTHREAD_MUTEX_INITIALIZER;
unordered_map<string, string> MimeType::mime;

string MimeType::getMime(const string &suffix)
{
    if(mime.size() == 0)
    {
        pthread_mutex_lock(&lock);
        if(mime.size() == 0)
        {
            mime[".html"] = "text/html";
            mime[".avi"] = "video/x-msvideo";
            mime[".bmp"] = "image/bmp";
            mime[".c"] = "text/plain";
            mime[".doc"] = "application/msword";
            mime[".gif"] = "image/gif";
            mime[".gz"] = "application/x-gzip";
            mime[".htm"] = "text/html";
            mime[".ico"] = "application/x-ico";
            mime[".jpg"] = "image/jpeg";
            mime[".png"] = "image/png";
            mime[".txt"] = "text/plain";
            mime[".mp3"] = "audio/mp3";
            mime["default"] = "text/html";
        }
        pthread_mutex_unlock(&lock);
    }
    if (mime.find(suffix) != mime.end())
        return mime[suffix];
    else
        return mime["default"];
}

priority_queue<shared_ptr<mytimer>, deque<shared_ptr<mytimer>>, timerCmp> myTimeQueue;

Solver::Solver(): now_read_pos(0), state(), h_state(h_start), keep_alive(false), againTimes(0)
{
    cout << "Solver()" << endl;
}

Solver::Solver(int _epollfd, int _fd, std::string _path): 
now_read_pos(0), state(), h_state(h_start), keep_alive(false), againTimes(0), 
path(_path), fd(_fd), epollfd(_epollfd)
{
    cout << "Solver()" << endl;
}

Solver::~Solver()
{
    cout << "~Solver()" << endl;
    close(fd);
}

void Solver::addTimer(shared_ptr<mytimer> mtimer)
{
    timer = mtimer;
}

int Solver::getFd()
{
    return fd;
}

void Solver::setFd(int _fd)
{
    fd = _fd;
}

void Solver::reset()
{
    againTimes = 0;
    content.clear();
    file_name.clear();
    path.clear();
    now_read_pos = 0;
    state = STATE_PARSE_URI;
    h_state = h_start;
    headers.clear();
    keep_alive = false;
    if(timer.lock())
    {
        shared_ptr<mytimer> my_timer(timer.lock());
        my_timer->clearReq();
        timer.reset();
    }
}

void Solver::seperateTimer()
{
    if(timer.lock())
    {
        shared_ptr<mytimer> my_timer(timer.lock());
        my_timer->clearReq();
        timer.reset();
    }
}

void Solver::handleRequest()
{
    char buff[MAX_BUFF];
    bool isError = false;
    while(true)
    {
        int read_num = rio_readn(fd, buff, MAX_BUFF);
        if (read_num < 0)
        {
            perror("1");
            isError = true;
            break;
        }
        else if ( read_num == 0)
        {
            //读不到数据，Request Aborted。
            perror("read_num == 0");
            if(errno == EAGAIN)
            {
                if(againTimes > AGAIN_MAX_TIMES){
                    isError = true;
                }
                else{
                    ++againTimes;
                }
            }
            else if ( errno != 0){
                isError = true;
            }
            break;
        }
        string now_read(buff, buff + read_num);
        content += now_read;

        if (state == STATE_PARSE_URI)
        {
            int flag = this->parse_URI();
            if (flag == PARSE_URI_AGAIN){
                break;
            }
            else if ( flag == PARSE_URI_ERROR){
                perror("2");
                isError = true;
                break;
            }
        }
        if (state == STATE_PARSE_HEADERS)
        {
            int flag = this->parse_Headers();
            if ( flag == PARSE_HEADER_ERROR){
                break;
            }
            else if (flag == PARSE_HEADER_ERROR){
                perror("3");
                isError = true;
                break;
            }
            if(method == METHOD_POST)
            {
                state = STATE_RECV_BODY;
            }
            else {
                state = STATE_ANALYSIS;
            }
        }
        if (state == STATE_RECV_BODY)
        {
            int content_length = -1;
            if ( headers.find("Content-length") != headers.end())
            {
                content_length = stoi(headers["Content-length"]);
            }
            else{
                isError = true;
                break;
            }

            if (content.size() < content_length)
                continue;
            state = STATE_ANALYSIS;
        }
        if (state == STATE_ANALYSIS)
        {
            int flag = this->analysisRequest();
            if (flag < 0){
                isError = true;
                break;
            }
            else if ( flag == ANALYSIS_SUCCESS){
                state = STATE_FINISH;
                break;
            }
            else {
                isError = true;
                break;
            }
        }
    }
    if (isError)
    {
        //delete this;
        return;
    }
    // 加入epoll继续
    if (state == STATE_FINISH)
    {
        if (keep_alive)
        {
            //printf("ok\n");
            this->reset();
        }
        else
        {
            //delete this;
            return;
        }
    }
    //加入定时器，在添加EPOLLIN事件前加入；
    shared_ptr<mytimer> mtimer(new mytimer(shared_from_this(), 500));
    this->addTimer(mtimer);
    {
        MutexLockGuard lock;        //RAII锁
        myTimeQueue.push(mtimer);
    }

    __uint32_t _epoll_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
    int ret = Epoll::epoll_mod(fd, shared_from_this(), _epoll_event);
    cout << "shared_from_this().use_count() == " << shared_from_this().use_count() << endl;
    if (ret < 0)
    {
        //错误
        return ;
    }
}

int Solver::parse_URI()
{
    string &str = content;
    //
    int pos = str.find('\r', now_read_pos);
    if (pos < 0){
        return PARSE_URI_AGAIN;
    }
    //去掉请求行，省空间。
    string request_line = str.substr(0, pos);
    if(str.size() > pos + 1){
        str = str.substr(pos + 1);
    }
    else
    {
        str.clear();
    }
    pos = request_line.find("GET");
    if (pos < 0){
        pos = request_line.find("POST");
        if (pos < 0){
            return PARSE_URI_ERROR;
        }
        else {
            method = METHOD_POST;
        }
    }
    else{
        method = METHOD_GET;
    }

    //analysis the request file
    pos = request_line.find("/", pos);
    if (pos < 0){
        //必须设置 "/"
        return PARSE_URI_ERROR;
    }
    else{
        int _pos = request_line.find(' ', pos);
        if (_pos < 0){
            return PARSE_URI_ERROR;
        }
        else {
            if(_pos - pos > 1){
                file_name = request_line.substr(pos + 1, _pos - pos - 1);
                int __pos = file_name.find('?');
                if (__pos >= 0){
                    file_name = file_name.substr(0, __pos);
                }
            }
            else
            {
                //默认选项
                file_name = "index.html";
            }
        }
        pos = _pos;
    }
    cout << "file_name: " << file_name << endl;
    //HTTP version
    pos = request_line.find("/", pos);
    if (pos < 0)
    {
        return PARSE_URI_ERROR;
    }
    else {
        if(request_line.size() - pos <= 3){
            return PARSE_URI_ERROR;
        }
        else {
            string ver = request_line.substr(pos + 1, 3);
            if (ver == "1.0")
                HTTPversion = HTTP_10;
            else if (ver == "1.1")
                HTTPversion = HTTP_11;
            else 
                return PARSE_URI_ERROR;
        }
    }
    state = STATE_PARSE_HEADERS;
    return PARSE_URI_SUCCESS;
}

int Solver::parse_Headers()
{
    string &str = content;
    int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
    int now_read_line_begin = 0;
    bool notFinish = true;
    for(int i = 0; i < str.size() && notFinish; ++i){
        switch(h_state)
        {
            case h_start:
            {
                if (str[i] == '\n' || str[i] == '\r')
                    break;
                h_state = h_key;
                key_start = i;
                now_read_line_begin = i;
                break;
            }
            case h_key:
            {
                if(str[i] == ':')
                {
                key_end = i;
                if (key_end - key_start <= 0){
                    return PARSE_HEADER_ERROR;
                }
                h_state = h_colon;
                }
                else if (str[i] == '\n' || str[i] == '\r'){
                    return PARSE_HEADER_ERROR;
                }
                break;
            }
            case h_colon:
            {
                if(str[i] == ' '){
                    h_state = h_spaces_after_colon;
                }
                else {
                    return PARSE_HEADER_ERROR;
                }
                break;
            }
            case h_space_after_colon:
            {
                h_state = h_value;
                value_start = i;
                break;
            }
        }
    }
}

