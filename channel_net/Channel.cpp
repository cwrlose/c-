//
// Created by Administrator on 2025/4/7.
//

#include "Channel.h"
//初始化
Channel::Channel(Epoll *_ep, int _fd):ep(_ep),fd(_fd),events(0),revents(0),inEpoll(false){
}
//
void Channel::enableReading() {
    events=EPOLLIN|EPOLLET;
    ep->updataChannel(this);//添加事件表
}
int Channel::getFd() {
    return  fd;
}
bool Channel::getInEpoll() {
    return inEpoll;
}
uint32_t Channel::getRevents(){
    return revents;
}
uint32_t Channel::getEvents() {
    return events;
}

void Channel::setInEpoll() {
    inEpoll=true;
}
void Channel::setRevents(uint32_t _ev){
    revents = _ev;
}




