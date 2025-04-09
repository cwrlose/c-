//
// Created by Administrator on 2025/4/2.
//
#include<sys/socket.h>
#include "Socket.h"
#include "util.h"
#include"InetAddress.h"
#include <unistd.h>
#include<fcntl.h>
Socket::Socket():fd(-1){
    fd=socket(AF_INET,SOCK_STREAM,0);
    errif(fd==-1,"create socket error");
}
Socket::Socket(int _fd):fd(_fd) {//从其他地方获取的文件描述符
    errif(fd==-1,"socket create error");
}
Socket::~Socket() {
    if(fd!=-1) {
        close(fd);
        fd=-1;
    }

}
/*
* :: 的作用：明确指定调用的是全局作用域中的函数，而不是类成员函数。
为什么需要 ::：为了避免命名冲突，确保调用的是正确的函数。
 */
void Socket::bind(InetAddress*addr) {
    errif(::bind(fd,(sockaddr*)&addr->sockaddr,addr->addr_len),"socket bind error");
}
void Socket::listen() {
    errif(::listen(fd,SOMAXCONN),"socket listen error");
}
void Socket::setnonblocking() {
    fcntl(fd,F_SETFL,fcntl(fd,F_GETFD)|O_NONBLOCK);
}
int Socket::accept(InetAddress* addr) {
    int clnt_sockfd=::accept(fd,(sockaddr*)&addr->sockaddr,&addr->addr_len);
    errif(clnt_sockfd==-1,"socket accept error");
    return clnt_sockfd;
}


int Socket::Getfd() {
    return fd;
}







