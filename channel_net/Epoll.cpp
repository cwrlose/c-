//
// Created by Administrator on 2025/4/2.
//

#include "Epoll.h"

#include <strings.h>
#include<sys/epoll.h>
#include "util.h"
#include <unistd.h>
#include"Channel.h"

Epoll::Epoll():epfd(-1), events(nullptr)
{
    epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");
    events = new epoll_event[MAX_EVENTS];//Type* pointer = new Type[size];
    bzero(events, sizeof(*events) * MAX_EVENTS);
}
Epoll::~Epoll() {
    if(epfd!=-1) {
        close(epfd);
        epfd=-1;
    }
    delete [] events;
}
void Epoll::addFd(int fd,uint32_t op) {
    struct epoll_event ev;
    bzero(&ev,sizeof(ev));
    ev.data.fd= fd;
    ev.events=op;
    errif(epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev)==-1,"epoll add event error");
}
//直接返回就绪事件
/*
std::vector<epoll_event> Epoll::poll(int timeout) {
    std::vector<epoll_event>active_events;
    int nfds=epoll_wait(epfd,events,MAX_EVENTS,timeout);
    errif(nfds==-1,"epoll wait error");
    for(int i=0;i<nfds;i++) {
        active_events.push_back(events[i]);
    }
    return active_events;
}*/
void Epoll::updataChannel(Channel *channel) {
    int fd=channel->getFd();
    struct epoll_event ev;
    bzero(&ev,sizeof(ev));

    ev.data.ptr=channel;
    ev.events=channel->getEvents();

    if (!channel->getInEpoll()) {
        errif(epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev)==-1,"EPOLL add fd error");
        channel->setInEpoll();
    }
    else {
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
        //该操作是对已经注册到 epoll 实例里的文件描述符所监听的事件进行修改。当程序运行时，依据业务需求要动态改变文件描述符的监听事件类型，就会使用它。

    }
}
std::vector<Channel *> Epoll::poll(int timeout) {
    std::vector<Channel*>activate_channels;

    int nepfd=epoll_wait(epfd,events,MAX_EVENTS,timeout);

    for (int i=0;i<nepfd;i++) {
        Channel*ch=(Channel*)events[i].data.ptr;
        ch->setRevents(events[i].events);
        activate_channels.push_back(ch);
    }
    return  activate_channels;
}






