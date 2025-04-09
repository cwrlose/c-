//
// Created by Administrator on 2025/4/2.
//

#ifndef EPOLL_H
#define EPOLL_H
#include<sys/epoll.h>
#include<vector>
#include"util.h"
#include<string.h>
#include"Channel.h"

#define MAX_EVENTS 1000
class Socket;
class Channel;
class Epoll {
private:
    int epfd;
    struct epoll_event* events;

public:
    Epoll();
    ~Epoll();

    void addFd(int fd,uint32_t op);
    //std::vector<epoll_event> poll(int timeout = -1);
    void updataChannel(Channel*);
    std::vector<Channel*> poll(int timeout = -1);
};

#endif //EPOLL_H
