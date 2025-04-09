//
// Created by Administrator on 2025/4/2.
//
#include"util.h"
#include"Socket.h"
#include"InetAddress.h"
#include"Epoll.h"
#include<vector>
#include<iostream>
#include<unistd.h>
#include"Channel.h"

int main() {
    //灵活性：可以按需创建和销毁对象，而不需要在编译时确定对象的数量和生命周期。
    //避免栈溢出：对于大型对象或大量对象，动态分配可以避免栈溢出问题
    Socket *serv_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    //绑定
    serv_sock->bind(serv_addr);
    serv_sock->listen();

    Epoll *ep = new Epoll();
    serv_sock->setnonblocking();

    Channel* ser_channel=new Channel(ep,serv_sock->Getfd());
    ser_channel->enableReading();

    while(true) {
    std::vector<Channel*>activate_channels=ep->poll(-1);
        int nfds=activate_channels.size();
        for(int i=0;i<nfds;i++) {
            //连接
            if(activate_channels[i]->getFd()==serv_sock->Getfd()) {
                InetAddress* clnt_addr= new InetAddress();
                Socket *clnt_sock=new Socket(serv_sock->accept(clnt_addr));
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->Getfd(), inet_ntoa(clnt_addr->sockaddr.sin_addr), ntohs(clnt_addr->sockaddr.sin_port));

                //注册事件
                clnt_sock->setnonblocking();
                Channel*clnt_channel=new Channel(ep,clnt_sock->Getfd());
                clnt_channel->enableReading();
            }
            else if(activate_channels[i]->getEvents()&EPOLLIN) {
                handleReadEvent(activate_channels[i]->getFd());
            }
            else{         //其他事件，之后的版本实现
                printf("something else happened\n");
            }
        }
    }
    delete serv_sock;
    delete serv_addr;
    delete ser_channel;
    return 0;
}


