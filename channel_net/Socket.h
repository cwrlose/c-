//
// Created by Administrator on 2025/4/2.
//

#ifndef SOCKET_H
#define SOCKET_H


class InetAddress;
class Socket {
private:
    int fd;

public:
    Socket();
    Socket(int);
    ~Socket();
    void bind(InetAddress*);
    void listen();
    void setnonblocking();

    int accept(InetAddress*);

    int Getfd();

};



#endif //SOCKET_H
