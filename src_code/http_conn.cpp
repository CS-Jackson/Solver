#include "http_conn.h"
#include "Epoll.h"
#include "rio.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <unordered_map>
#include <queue>
#include <fcntl.h>

#include <iostream>
using namespace std;

// pthread_mutex_t MutexLockGuard::lock = PTHREAD_MUTEX_INITIALIZER; //In locker.cpp
// pthread_mutex_t MimeType::lock = PTHREAD_MUTEX_INITIALIZER;
pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
unordered_map<string, string> MimeType::mime;

void MimeType::init()
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

std::string MimeType::getMime(const std::string &suffix)
{
    // if(mime.size() == 0)
    // {
    //     pthread_mutex_lock(&lock);
    //     if(mime.size() == 0)
    //     {
            
    //     }
    //     pthread_mutex_unlock(&lock);
    // }
    pthread_once(&once_control, MimeType::init);
    if (mime.find(suffix) != mime.end())
        return mime[suffix];
    else
        return mime["default"];
}

// priority_queue<shared_ptr<mytimer>, deque<shared_ptr<mytimer>>, timerCmp> myTimerQueue;

Solver::Solver():
now_read_pos(0), state(STATE_PARSE_URI), h_state(h_start), keep_alive(false), 
isAbleRead(true), isAbleWrite(false),
events(0), error(false)
{
    // againTimes(0),
    // cout << "Solver()" << endl;
}

Solver::Solver(int _epollfd, int _fd, std::string _path): 
path(_path), fd(_fd), epollfd(_epollfd), now_read_pos(0), state(STATE_PARSE_URI), h_state(h_start), keep_alive(false),
isAbleRead(true), isAbleWrite(false), events(0), error(false)
{
    // DB.initDB("localhost", "root", "csj84377532", "test");
    // againTimes(0),
    // cout << "Solver()" << endl;
}

Solver::~Solver()
{
    // cout << "~Solver()" << endl;
    close(fd);
}

// void Solver::addTimer(shared_ptr<mytimer> mtimer)
// {
//     timer = mtimer;
// }
void Solver::linkTimer(shared_ptr<mytimer> mtimer)
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
    // againTimes = 0;
    // content.clear();
    inBuffer.clear();
    file_name.clear();
    path.clear();
    now_read_pos = 0;
    state = STATE_PARSE_URI;
    h_state = h_start;
    headers.clear();
    // keep_alive = false; 
    if(timer.lock())
    {
        shared_ptr<mytimer> my_timer(timer.lock());
        my_timer->clearReq();   //清除timer里面对HTTP类的智能指针
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

void Solver::handleRead()
{
    do
    {
        int read_num = rio_readn(fd, inBuffer);
        // printf()
        // cout << inBuffer << endl;
        if(read_num < 0){
            perror("1");
            error = true;
            handleError(fd, 400, "Bad Request");
            break;
        }
        else if (read_num == 0)
        {
            //上一轮还分是EINTR，EAGAIN，后面发现都是属于报错，所以直接返回就可以。
            error = true;
            break;
        }
        if(state == STATE_PARSE_URI)
        {
            int flag = this->parse_URI();
            if(flag == PARSE_URI_AGAIN)
            {
                break;
            }
            else if (flag == PARSE_URI_ERROR)
            {
                perror("2");
                error = true;
                handleError(fd, 400, "Bad Request");
                break;
            }
            else 
                state = STATE_PARSE_HEADERS;
        }
        if (state == STATE_PARSE_HEADERS)
        {
            int flag = this->parse_Headers();
            if(flag == PARSE_HEADER_AGAIN){
                break;
            }
            // else if((flag == PARSE_HEADER_ERROR) && ((errno != EAGAIN)))
            else if(flag == PARSE_HEADER_ERROR) 
            {
                perror("3");
                error = true;
                handleError(fd, 400, "Bad Request");
                break;
            }
            if(method == METHOD_POST)
            {
                state = STATE_RECV_BODY;
            }
            else
            {
                state = STATE_ANALYSIS;
            }
        }
        if (state == STATE_RECV_BODY)
        {
            int content_length = -1;
            if(headers.find("Content-Length") != headers.end()){
                content_length = stoi(headers["Content-Length"]);
            }
            else{
                error = true;
                handleError(fd, 400, "Bad Request because of lack of argument (Content-Length)");
                break;
            }
            if(inBuffer.size() < content_length){
                break;
            }
            state = STATE_ANALYSIS;
        }
        if (state == STATE_ANALYSIS)
        {
            int flag = this->analysisRequest();
            if(flag == ANALYSIS_SUCCESS)
            {
                state = STATE_FINISH;
                break;
            }
            else {
                error = true;
                break;
            }
        }
    }while(false);

    if (!error)
    {
        if(outBuffer.size() > 0){
            events |= EPOLLOUT;
        }
        if(state == STATE_FINISH){
            // cout << "keep_alive=" << keep_alive << endl;
            if(keep_alive)
            {
                this->reset();
                events |= EPOLLIN;
            }
            else{
                return ;
            }
        }
        else{
            events |= EPOLLIN;
        }
    }
}
void Solver::handleWrite()
{
    if(!error)
    {
        if(rio_writen(fd, outBuffer) < 0)
        {
            perror("written");
            events = 0;
            error = true;
        }
        else if(outBuffer.size() > 0){
            events |= EPOLLOUT; //没写完？
        }
    }
}

void Solver::handleConn()
{
    if(!error)
    {
        if(events != 0)
        {
            //添加时间信息
            int timeout = 2000;
            if(keep_alive){
                timeout = 5 * 60 * 1000;
            }
            isAbleRead = false;
            isAbleWrite = false;
            Epoll::add_timer(shared_from_this(), timeout);
            if ( (events & EPOLLIN) && (events & EPOLLOUT))
            {
                events = __uint32_t(0);
                events |= EPOLLOUT;
            }
            events |= (EPOLLET | EPOLLONESHOT);
            __uint32_t _events = events;
            events = 0;
            if(Epoll::epoll_mod(fd, shared_from_this(), _events) < 0){
                printf("Epoll::epoll_mod error\n");
            }
        }
        else if (keep_alive)
        {
            events |= (EPOLLIN | EPOLLET | EPOLLONESHOT);
            int timeout = 5 * 60 * 1000;
            isAbleRead = false;
            isAbleWrite = false;
            Epoll::add_timer(shared_from_this(), timeout);
            __uint32_t _events = events;
            events = 0;
            if (Epoll::epoll_mod(fd, shared_from_this(), _events) < 0)
            {
                printf("Epoll::epoll_mod error\n");
            }
        }
    }
}

int Solver::parse_URI()
{
    string &str = inBuffer;
    cout << inBuffer << endl;
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
                file_name = "Webpage/index.html";
            }
        }
        pos = _pos;
    }
    // scout << "file_name: " << file_name << endl;
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
    // state = STATE_PARSE_HEADERS;//调到handleRequest里改变状态
    return PARSE_URI_SUCCESS;
}

int Solver::parse_Headers()
{
    string &str = inBuffer;
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
            case h_spaces_after_colon:
            {
                h_state = h_value;
                value_start = i;
                break;
            }
            case h_value:
            {
                if(str[i] == '\r')
                {
                    h_state = h_CR;
                    value_end = i;
                    if (value_end - value_start <= 0){
                        return PARSE_HEADER_ERROR;
                    }
                }
                else if (i - value_start > 255){//?
                    return PARSE_HEADER_ERROR;
                }
                break;
            }
            case h_CR:
            {
                if(str[i] == '\n'){
                    h_state = h_LF;
                    string key(str.begin() + key_start, str.begin() + key_end);
                    string value(str.begin() + value_start, str.begin() + value_end);
                    headers[key] = value;
                    now_read_line_begin = i;
                }
                else {
                    return PARSE_HEADER_ERROR;
                }
                break;
            }
            case h_LF:
            {
                if (str[i] == '\r')
                {
                    h_state = h_end_CR;
                }
                else{
                    key_start = i;
                    h_state = h_key;
                }
                break;
            }
            case h_end_CR:
            {
                if(str[i] == '\n'){
                    h_state = h_end_LF;
                }
                else{
                    return PARSE_HEADER_ERROR;
                }
                break;
            }
            case h_end_LF:
            {
                notFinish = false;
                key_start = i;
                now_read_line_begin = i;
                break;
            }
        }
    }
    if(h_state == h_end_LF){
        str = str.substr(now_read_line_begin);
        return PARSE_HEADER_SUCCESS;
    }
    str = str.substr(now_read_line_begin);
    return PARSE_HEADER_AGAIN;
}

int Solver::analysisRequest()
{
    if(method == METHOD_POST)
    {
        //get content
        //char header[MAX_BUFF];
        string header;
        // sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
        header += string("HTTP/1.1 200 OK\r\n");
        if(headers.find("Connection") != headers.end() && headers["Connection"] == "keep-alive"){
            keep_alive = true;
            // sprintf(header, "%sConnection: keep-alive\r\n", header);
            // sprintf(header, "%sKeep-alive: timeout=%d\r\n", header, EPOLL_WAIT_TIME);
            header += string("Connection: keep-alive\r\n") + "Keep-Alive: timeout=" + to_string(5 * 60 * 1000) + "\r\n";
        }
        int dot_pos = file_name.find('.');
        string filetype;
        if(dot_pos < 0){
            filetype = MimeType::getMime("default");//.c_str();
        }
        else{
            filetype = MimeType::getMime(file_name.substr(dot_pos));//.c_str();
        }
        struct stat sbuf;

        if(stat(file_name.c_str(), &sbuf) < 0){
            header.clear();
            handleError(fd, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }
        header += "Content-type: " + filetype + "\r\n";
        header += "Content-Length: " + to_string(sbuf.st_size) + "\r\n";
        header += "\r\n";   //头部结尾
        outBuffer += header;
        int src_fd = open(file_name.c_str(), O_RDONLY, 0);
        char *src_addr = static_cast<char *>(mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
        close(src_fd);

        // printf(src_addr);
        
        outBuffer += src_addr;
        munmap(src_addr, sbuf.st_size);
        // cout << "content=" << content << endl;
        //
        // char send_content[] = "I have receiced this.";
        // sprintf(header, "%sContent-Length: %zu\r\n", header, strlen(send_content));
        // sprintf(header, "%s\r\n", header);
        int length = stoi(headers["Content-Length"]);
        // size_t send_len = (size_t)rio_writen(fd, header, strlen(header));
        // if(send_len != strlen(header)){
        //     perror("Send header failed");
        //     return ANALYSIS_ERROR;
        // }
        // send_len = (size_t)rio_writen(fd, send_content, strlen(send_content));
        // if(send_len != strlen(send_content)){
        //     perror("Send content failed");
        //     return ANALYSIS_ERROR;
        // }
        // cout << "content size == " << content.size() << endl;
        // vector<char> data(content.begin(), content.end());
        // vector<char> data(inBuffer.begin(), inBuffer.begin() + length);
        // Mat src = imdecode(data, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
        // imwrite("receive.bmp", src);
        // Mat res = stitch(src);
        // vector<uchar> data_encode;
        // imencode(".png", res, data_encode);
        // header += string("Content-Length: ") + to_string(data_encode.size()) + "\r\n\r\n";
        // outBuffer += header + string(data_encode.begin(), data_encode.end());
        string POST_BODY(inBuffer.begin(), inBuffer.begin()+length);
        cout << "POST_BODY: " << POST_BODY << endl;
        string::iterator POSTbegin = POST_BODY.begin(), key = POST_BODY.begin(), value = POST_BODY.begin();
        unordered_map<string, string> RECV_BODY;
        for(string::iterator iter1 = POST_BODY.begin(); iter1 != POST_BODY.end(); iter1++)
        {
            if(*iter1 == '=') {
                key = iter1;
            }
            if(*iter1 == '&') {
                string key_s(POSTbegin, key);
                string value_s(key + 1, iter1);
                RECV_BODY[key_s] = value_s;
                POSTbegin = iter1 + 1;
            }
            if(iter1 + 1 == POST_BODY.end()) {
                string key_s(POSTbegin, key);
                string value_s(key + 1, iter1 + 1);
                RECV_BODY[key_s] = value_s;
                POSTbegin = iter1 + 1;
            }
        }
        for(unordered_map<string, string>::iterator ss = RECV_BODY.begin(); ss != RECV_BODY.end(); ss++)
        {
            cout << "first: " << (*ss).first << " " << "second: " << (*ss).second << endl;
        }
        cout << "\r\n" << endl;
        inBuffer = inBuffer.substr(length);
        return ANALYSIS_SUCCESS;
    }
    else if (method == METHOD_GET)
    {
        // char header[MAX_BUFF];
        string header;
        // sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
        header += "HTTP/1.1 200 OK\r\n";
        if(headers.find("Connection") != headers.end() && headers["Connection"] == "keep-alive")
        {
            keep_alive = true;
            // sprintf(header, "%sConnection: keep-alive\r\n", header);
            // sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, EPOLL_WAIT_TIME);
            header += string("Connection: keep-alive\r\n") + "Keep-Alive: timeout=" + to_string(5 * 60 * 1000) + "\r\n";
        }
        int dot_pos = file_name.find('.');
        //const char *filetype;
        string filetype;
        if(file_name.substr(dot_pos) == ".cgi") {
            cout << "Exec the cig file" << endl;
            dup2(fd, 1);
            // close(fd);
            execlp(file_name.c_str(), NULL);
            return ANALYSIS_SUCCESS;
        }
        if(dot_pos < 0){
            filetype = MimeType::getMime("default");//.c_str();
        }
        else{
            filetype = MimeType::getMime(file_name.substr(dot_pos));//.c_str();
        }
        struct stat sbuf;
        if(stat(file_name.c_str(), &sbuf) < 0){
            header.clear();
            handleError(fd, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }
        // sprintf(header, "%sContent-type: %s\r\n", header, filetype);
        // //return size of file.
        // sprintf(header, "%sContent-Length: %ld\r\n", header, sbuf.st_size);

        // sprintf(header, "%s\r\n", header);
        header += "Content-type: " + filetype + "\r\n";
        header += "Content-Length: " + to_string(sbuf.st_size) + "\r\n";
        header += "\r\n";   //头部结尾
        outBuffer += header;
        // size_t send_len = (size_t)rio_writen(fd, header, strlen(header));
        // if(send_len != strlen(header)){
        //     if (send_len == -1)
        //     {
        //         cout << "Send file failed because client closed" << endl;
        //         state = STATE_CLOSE;
        //         return ANALYSIS_ERROR;
        //     }
        //     perror("Send header failed");
        //     return ANALYSIS_ERROR;
        // }
        int src_fd = open(file_name.c_str(), O_RDONLY, 0);
        char *src_addr = static_cast<char *>(mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
        close(src_fd);

        //send file and check complete
        outBuffer += src_addr;
        // send_len = rio_writen(fd, src_addr, sbuf.st_size);//return a size_t var.
        // if(send_len != sbuf.st_size){//the signed int to size_t maybe wrong?
        //     if (send_len == -1)
        //     {
        //         cout << "Send file failed because client closed" << endl;
        //         state = STATE_CLOSE;
        //         return ANALYSIS_ERROR;
        //     }
        //     perror("Send file failed");
        //     return ANALYSIS_ERROR;
        // }
        cout << outBuffer << endl;

        munmap(src_addr, sbuf.st_size);
        return ANALYSIS_SUCCESS;
    }
    else 
        return ANALYSIS_ERROR;
}

void Solver::handleError(int fd, int err_num, string short_msg)
{
    short_msg = " " + short_msg;
    char send_buff[MAX_BUFF];
    string body_buff, header_buff;
    body_buff += "<html><title>Solver Error</title>";
    body_buff += "<body bgcolor=\"ffffff\">";
    body_buff += to_string(err_num) + short_msg;
    body_buff += "<hr><em> Jackson's Web Server</em>\n</body></html>";

    header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
    header_buff += "Content-type: text/html\r\n";
    header_buff += "Connection: close\r\n";
    header_buff += "Content-Length: " + to_string(body_buff.size()) + "\r\n";
    header_buff += "\r\n";
    sprintf(send_buff, "%s", header_buff.c_str());
    rio_writen(fd, send_buff, strlen(send_buff));
    sprintf(send_buff, "%s", body_buff.c_str());
    rio_writen(fd, send_buff, strlen(send_buff));
}

void Solver::disableReadAndWrite()
{
    isAbleRead = false;
    isAbleWrite = false;
}

void Solver::enableRead()
{
    isAbleRead = true;
}

void Solver::enableWrite()
{
    isAbleWrite = true;
}
bool Solver::canRead()
{
    return isAbleRead;
}

bool Solver::canWrite()
{
    return isAbleWrite;
}

