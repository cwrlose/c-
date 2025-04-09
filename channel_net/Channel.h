//
// Created by Administrator on 2025/4/7.
//

#ifndef CHANNEL_H
#define CHANNEL_H

#include"Epoll.h"
/*
 * 如果一个服务器同时提供不同的服务，如HTTP、FTP等，那么就算文件描述符上发生的事件都是可读事件，不同的连接类型也将决定不同的处理逻辑，仅仅通过一个文件描述符来区分显然会很麻烦，
 * 我们更加希望拿到关于这个文件描述符更多的信息。
 * 原于data是一个指针
 */
class Epoll;
class Channel {
private:
 Epoll *ep;//事件表
 int fd;//监听的文件描述符
 uint32_t events;//events表示希望监听这个文件描述符的哪些事件
 uint32_t revents;//文件描述符正在发生的事件
 bool inEpoll;//当前的channel
public:
 //构造
 Channel(Epoll *_ep, int _fd);
 ~Channel();

 //功能
 void enableReading();
 //属性
 int getFd();
 uint32_t getEvents();
 uint32_t getRevents();
 bool getInEpoll();
 void setInEpoll();
 void setRevents(uint32_t);
};



#endif //CHANNEL_H
