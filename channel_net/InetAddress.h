//
// Created by Administrator on 2025/4/2.
//

#ifndef LNETADRESS_H
#define LNETADRESS_H


#include <arpa/inet.h>
class InetAddress {
public:
    struct sockaddr_in sockaddr;
    socklen_t addr_len;
    InetAddress();
    InetAddress(const char* ip,uint16_t port);
    ~InetAddress();
};



#endif //LNETADRESS_H
