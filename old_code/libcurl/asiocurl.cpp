#include "boost/asio.hpp"
#include <chrono>
#include <curl/curl.h>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
//#include "asio.hpp"

using namespace std;
using std::placeholders::_1;

class Session;

class MultiInfo
{
    friend class Session;
    CURLM *multi_;
    int still_running_;
    boost::asio::io_context ioc_;
    boost::asio::steady_timer timer_;
    unordered_map<curl_socket_t, Session*> socket_map_;
public:
    static MultiInfo* instance() {
        static MultiInfo *p = new MultiInfo;
        return p;
    }
    void run() {
        ioc_.run();
    }
private:
    static int socket_callback(CURL *easy, curl_socket_t s, int what, void *userp, void* socketp);
    static int timer_callback(CURLM *multi, long timeout_ms, void *userp) {
        cout << "---------->timeout=" << timeout_ms << "\n";
        boost::asio::steady_timer &timer = MultiInfo::instance()->timer_;
        timer.cancel();
        if(timeout_ms > 0) {
            timer.expires_from_now(std::chrono::milliseconds(timeout_ms));
            timer.async_wait(std::bind(&asio_timer_callback, _1));
        }
        else if(timeout_ms == 0) {
            boost::system::error_code error;
            asio_timer_callback(error);
        }
        return 0;
    }

private:
    static void asio_timer_callback(const boost::system::error_code &error) {
        if(!error) {
            CURLMcode mc = curl_multi_socket_action(MultiInfo::instance()->multi_, CURL_SOCKET_TIMEOUT, 0, 
                                                    &(MultiInfo::instance()->still_running_));
            if(mc != CURLM_OK) {
                cout << "curl_multi_socket_action error, mc=" << mc << "\n";
            }
            check_multi_info();
        }
    }

    static void asio_socket_callback(const boost::system::error_code &ec, CURL* easy, curl_socket_t s, 
                                    int what, Session *session);
private:
    static void check_multi_info();

private:
    MultiInfo() : still_running_(0), ioc_(1), timer_(ioc_) {
        multi_ = curl_multi_init();
        curl_multi_setopt(multi_, CURLMOPT_SOCKETFUNCTION, MultiInfo::socket_callback);
        curl_multi_setopt(multi_, CURLMOPT_TIMERFUNCTION, MultiInfo::timer_callback);
    }
    MultiInfo(const MultiInfo&) = delete;
    MultiInfo(MultiInfo &&) = delete;
    MultiInfo & operator=(const MultiInfo &) = delete;
    MultiInfo & operator=(MultiInfo&&) = delete;
};

using FinishHttp = void(*)(const string &url, string && html);

//one http request and response
class Session
{
    friend class MultiInfo;
    CURL *easy_;
    string url_;
    string html_;
    char error_[CURL_ERROR_SIZE];
    FinishHttp finish_callback_;
    boost::asio::ip::tcp::socket *socket_;
    int newest_event_;
public:
    Session(const string url, FinishHttp finish_cb) 
        : easy_(nullptr), url_(url), finish_callback_(finish_cb), socket_(nullptr), newest_event_(0)
    {

    }
    ~Session() {
        if(easy_) {
            curl_multi_remove_handle(MultiInfo::instance()->multi_, easy_);
            curl_easy_cleanup(easy_);
        }
        delete socket_;
    }
    int Init() {
        easy_ = curl_easy_init();
        if(!easy_) {
            cout << "error to init easy\n";
            return -1;
        }
        curl_easy_setopt(easy_, CURLOPT_URL, url_.c_str());
        curl_easy_setopt(easy_, CURLOPT_WRITEFUNCTION,
                         Session::write_callback); // 某个连接收到数据了，需要保存数据在此回调函数中保存
        curl_easy_setopt(easy_, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(easy_, CURLOPT_VERBOSE, 1L); // curl输出连接中的更多信息
        curl_easy_setopt(easy_, CURLOPT_ERRORBUFFER, error_);
        curl_easy_setopt(easy_, CURLOPT_PRIVATE,
                         this); // 存储一个指针，通过curl_easy_getinfo函数的CURLINFO_PRIVATE参数来取
        curl_easy_setopt(easy_, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(easy_, CURLOPT_LOW_SPEED_TIME, 3L);
        curl_easy_setopt(easy_, CURLOPT_LOW_SPEED_LIMIT, 10L);
        curl_easy_setopt(easy_, CURLOPT_OPENSOCKETFUNCTION, Session::opensocket_callback);
        curl_easy_setopt(easy_, CURLOPT_OPENSOCKETDATA, this);
        curl_easy_setopt(easy_, CURLOPT_CLOSESOCKETFUNCTION, Session::closesocket_callback);
        curl_easy_setopt(easy_, CURLOPT_CLOSESOCKETDATA, this);
        CURLMcode mc = curl_multi_add_handle(MultiInfo::instance()->multi_, easy_);
        return mc;
    }
private:
    //recv data, save;
    static size_t write_callback(char *ptr, size_t size, size_t nmemb, void* userdata) {
        size_t written = size * nmemb;
        string str(static_cast<const char *>(ptr), written);
        Session *s = static_cast<Session*>(userdata);
        s->html_.append(str);
        return written;
    }

    //set callback for opening sockets
    static curl_socket_t opensocket_callback(void *clientp, curlsocktype purpose, struct curl_sockaddr* address) {
        curl_socket_t ret = CURL_SOCKET_BAD;
        Session *s = static_cast<Session*>(clientp);
        boost::asio::io_context &ioc_ = MultiInfo::instance()->ioc_;

        //ipv4
        if(purpose == CURLSOCKTYPE_IPCXN && address->family == AF_INET) {
            s->socket_ = new boost::asio::ip::tcp::socket(ioc_);
            boost::system::error_code ec;
            s->socket_->open(boost::asio::ip::tcp::v4(), ec);
            if(ec) {
                std::cout << "can't open file\n";
            }
            else {
                ret = s->socket_->native_handle();
                MultiInfo::instance()->socket_map_.insert(std::make_pair(ret, s));
            }
        }
        return ret;
    }

    //callback to socket close replacement function
    static int closesocket_callback(void* clientp, curl_socket_t item) {
        cout << "*******>closing fd=" << item << "\n";
        Session *s = static_cast<Session*>(clientp);
        boost::system::error_code ec;
        s->socket_->close(ec);
        MultiInfo::instance()->socket_map_.erase(item);
        return ec ? ec.value() : 0;
    }
};

//when socket has events, call this func.
int MultiInfo::socket_callback(CURL *easy, curl_socket_t s, int what, void *userp, void *socket)
{
    cout << "==========>socket_callback, s=" << s << ", what=" << what << "\n";
    Session* session = MultiInfo::instance()->socket_map_.find(s)->second;
    session->newest_event_ = what;
    if(what == CURL_POLL_REMOVE) {
        return 0;
    }
    if(what & CURL_POLL_IN) {
        session->socket_->async_wait(boost::asio::ip::tcp::socket::wait_read, std::bind(MultiInfo::asio_socket_callback, _1, easy, 5, CURL_POLL_OUT, session));
    }
    if(what & CURL_POLL_OUT) {
        session->socket_->async_wait(boost::asio::ip::tcp::socket::wait_write, std::bind(MultiInfo::asio_socket_callback, _1, easy, s, CURL_POLL_OUT, session));
    }
    return 0;
}

// 当libcurl中的某个socket需要监听的事件发生改变的时候，都会调用此函数；
// 比如说从没有变成可写、从可写变成可读等变化的时候都会调用此函数；
// 此函数的what传的是当前socket需要关注的全部事件，包括以前曾经通知过并且现在还需要的。
// 注意，因为asio每次wait都是一次性的，所以需要把以前的事件保存起来，每次wait之后都要再次调用wait。
inline void MultiInfo::asio_socket_callback(const boost::system::error_code &ec, CURL *easy, curl_socket_t s, int what, Session *session)
{
    cout << "..........>asio_socket_callback, ec=" << ec.value() << " , s=" << s << ", what=" << what << "\n";
    if(ec) {
        what = CURL_CSELECT_ERR;
    }
    MultiInfo *multi = MultiInfo::instance();
    CURLMcode rc = curl_multi_socket_action(multi->multi_, s, what, &multi->still_running_);
    if(rc != CURLM_OK) {
        cout << "curl_multi_socket_action error, rc=" << int(rc) << "\n";
    }

    check_multi_info();

    if(multi->still_running_ <= 0) {
        multi->timer_.cancel();
        return ;
    }

    // 继续监听相关的事件，因为asio的wait函数都是一次性的，而libcurl对同一种事件没有发生变化时不会再次通知。
    // 上面调用了curl_multi_socket_action函数，对应的fd可能已经删除了，所以先检查一下，确认未删除，然后再重新监听事件
    if( !ec && multi->socket_map_.find(s) != multi->socket_map_.end()) {
        if(what == CURL_POLL_IN && (session->newest_event_ & CURL_POLL_IN)) {
            session->socket_->async_wait(boost::asio::ip::tcp::socket::wait_read, std::bind(MultiInfo::asio_socket_callback, _1, easy, s, CURL_POLL_IN, session));
        }
        if(what == CURL_POLL_OUT && (session->newest_event_ & CURL_POLL_OUT)) {
            session->socket_->async_wait(boost::asio::ip::tcp::socket::wait_write, std::bind(MultiInfo::asio_socket_callback, _1, easy, s, CURL_POLL_OUT, session));
        }
    }
}

void MultiInfo::check_multi_info()
{
    int msgs_left;
    for(CURLMsg *msg = curl_multi_info_read(MultiInfo::instance()->multi_, &msgs_left); msg != nullptr; 
        msg = curl_multi_info_read(MultiInfo::instance()->multi_, &msgs_left)) {
        if(msg->msg == CURLMSG_DONE) {
            CURL *easy = msg->easy_handle;
            Session *s;
            curl_easy_getinfo(easy, CURLINFO_PRIVATE, &s);
            s->finish_callback_(s->url_, std::move(s->html_));
            delete s;
        }
    }
}

void Finish(const string& url, string &&html)
{
    cout << "finished, url=" << url << ", html:\n"; 
}

int main()
{
    string urls[] = {
        "https://ec.haxx.se/libcurl-drive-multi-socket.html",
    };

    for(const string &url : urls) {
        Session *session = new Session(url, Finish);
        int ret = session->Init();
        if(0 != ret) {
            cout << "init error, ret=" << ret << "\n";
            return -1;
        }
    }

    MultiInfo::instance()->run();
    return 0;
}