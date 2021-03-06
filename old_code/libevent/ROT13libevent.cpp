//For sockaddr_in
#include <netinet/in.h>
//For socket functions
#include <sys/socket.h>
//For fcntl
#include <fcntl.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_LINE 16384

void do_read(evutil_socket_t fd, short events, void *arg);
void do_write(evutil_socket_t fd, short events, void *arg);

char rot13_char(char c)
{
    //
    if( (c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'Z')) return c + 13;
    else if( (c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z')) return c - 13;
    else return c;
}

void readcb(struct bufferevent *bev, void *ctx)
{
    struct evbuffer *input, *output;
    char *line;
    size_t n;
    int i;
    input = bufferevent_get_input(bev);
    output = bufferevent_get_output(bev);
    while( (line = evbuffer_readln(input, &n, EVBUFFER_EOL_LF))) {
        for(i = 0; i < n; ++i) {
            line[i] = rot13_char(line[i]);
        }
        evbuffer_add(output, line, n);
        evbuffer_add(output, "\n", 1);
        free(line);
    }
    if(evbuffer_get_length(input) >= MAX_LINE) {
        //
        char buf[1024];
        while(evbuffer_get_length(input)) {
            int n = evbuffer_remove(input, buf, sizeof(buf));
            for(i = 0; i < n; ++i) buf[i] = rot13_char(buf[i]);
            evbuffer_add(output, buf, n);
        }
        evbuffer_add(output, "\n", 1);
    }
}

void errorcb(struct bufferevent *bev, short error, void *ctx)
{
    //connection has been closed, do any clean up here.
    if (error & BEV_EVENT_EOF) { } 
    //check errno to see what error occurred
    else if(error & BEV_EVENT_ERROR) { }
    //must be a timeout event handle, handle it.
    else if(error & BEV_EVENT_TIMEOUT) { }
    bufferevent_free(bev);
}

void do_accept(evutil_socket_t listener, short event, void *arg)
{
    struct event_base *base = (struct event_base *)arg;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr *)&ss, &slen);
    if(fd < 0) {
        perror("accept");
    }
    else if(fd > FD_SETSIZE) {
        close(fd);
    }
    else {
        struct bufferevent *bev;
        evutil_make_socket_nonblocking(fd);
        bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setcb(bev, readcb, NULL, errorcb, NULL);
        bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
        bufferevent_enable(bev, EV_READ | EV_WRITE);
    }
}

void run(void)
{
    evutil_socket_t listener;
    struct sockaddr_in sin;
    struct event_base *base;
    struct event *listener_event;

    base = event_base_new();
    if(!base) return ;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(40713);
    listener = socket(AF_INET, SOCK_STREAM, 0);
    evutil_make_socket_nonblocking(listener);

    if(bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return ;
    }
    if(listen(listener, 16) < 0) {
        perror("listen");
        return ;
    }

    listener_event = event_new(base, listener, EV_READ | EV_PERSIST, do_accept, (void*)base);
    event_add(listener_event, NULL);
    event_base_dispatch(base);
}
