//
// Created by Administrator on 2025/4/1.
//
#include <cstdio>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/unistd.h>
#include<stdlib.h>
#define READ_BUF 1024
void errif(bool condition,const char* errmsg) {
    if(condition) {
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}

int main(){
    int sockfd=socket(AF_INET,SOCK_STREAM,0);//自动选协议
    errif(sockfd==-1,"create socket error");

    struct sockaddr_in serv_addr;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(8888);

    errif(bind(sockfd,(sockaddr*)&serv_addr,sizeof(serv_addr)),"socket bind error");
    errif(listen(sockfd,SOMAXCONN),"socket listen error");

    struct sockaddr_in clnt_addr;
    socklen_t clnt_socklen=sizeof(clnt_addr);

    int clnt_fd=accept(sockfd,(sockaddr*)&clnt_addr,&clnt_socklen);
    printf("new client fd %d! IP: %s Port: %d\n", clnt_fd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

    while(true) {
        char buf[READ_BUF];
        bzero(&buf,sizeof(buf));

        size_t read_bytes=read(clnt_fd,buf,sizeof(buf));

        if(read_bytes>0) {
            printf("message from %d : %s\n",clnt_fd,buf);
            write(clnt_fd,buf,sizeof(buf));
        }
        else if(read_bytes==0) {
            printf("Server socket disconnected!\n");
            break;

        }
        else if (read_bytes == -1)
        {
            close(clnt_fd);
            errif(true, "socket read error");
        }

    }
    close(sockfd);
    return 0;
}
