#include <event2/event.h>
#include <iostream>
#include <string>
//test callback function, print hello world after 5 seconds.
void timer_cb(evutil_socket_t fd, short what, void *arg)
{
    auto str = static_cast<std::string *> (arg);
    std::cout << *str << std::endl;
}

int main()
{
    std::string str = "Hello, World";
    auto *base = event_base_new();

    struct timeval five_seconds = {1, 0};
    auto *ev = event_new(base, -1, EV_TIMEOUT, timer_cb, (void*)&str);

    event_add(ev, &five_seconds);
    event_base_dispatch(base);
    event_free(ev);
    event_base_free(base);

    return 0;
}
