//
// Created by Administrator on 2025/4/2.
//

#include "InetAddress.h"
#include<string.h>


InetAddress::InetAddress():addr_len(sizeof(sockaddr)) {
    bzero(&sockaddr,addr_len);
}
InetAddress::InetAddress(const char *ip, uint16_t port):addr_len(sizeof(sockaddr)){
    bzero(&sockaddr,addr_len);
    sockaddr.sin_addr.s_addr=inet_addr(ip);
    sockaddr.sin_port=htons(port);
    sockaddr.sin_family=AF_INET;
    addr_len=sizeof(sockaddr);//确保 addr_len 始终反映 sockaddr 的实际大小，防止潜在的错误
}
InetAddress::~InetAddress() {

}

