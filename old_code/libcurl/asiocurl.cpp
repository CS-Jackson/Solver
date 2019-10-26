#include "boost/asio.hpp"
#include <chrono>
#include <curl/curl.h>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;
using std::placeholders::_1;

class Session;

class MultiInfo
{
    friend class Session;
    CURLM *multi_;
    int still_running_;
    boost::asio::io_context ioc_;
};

int main()
{
    return 0;
}