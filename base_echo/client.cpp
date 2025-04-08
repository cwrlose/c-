
//
// Created by Administrator on 2025/4/1.
//
#include<sys/unistd.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include <cstdio>
#include <strings.h>
#include<cstring>

void errif(bool condition,const char* errmsg) {
    if(condition) {
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}
int main() {
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    errif(sockfd==-1,"create socket error");

    struct sockaddr_in clnt_addr;
    clnt_addr.sin_family=AF_INET;
    clnt_addr.sin_port=htons(8888);
    clnt_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

    errif(connect(sockfd,(sockaddr*)&clnt_addr,sizeof(clnt_addr))==-1,"socket connect error");

    while(true) {
        char buf[1024];
        bzero(&buf,sizeof(buf));
        //scanf("%s",buf);
        // 使用fgets代替scanf
        if (fgets(buf, sizeof(buf), stdin) == nullptr) {
            perror("fgets");
            break;
        }
        // 去掉换行符
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
        }

        size_t write_bytes=write(sockfd,buf,sizeof(buf));
        if (write_bytes == -1) {
            printf("Socket already disconnected, can't write any more!\n");
            break;
        }

        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("Message from server: %s\n", buf);
        } else if (read_bytes == 0) {
            printf("Server socket disconnected!\n");
            break;
        } else {
            printf("Socket read error\n");
            close(sockfd);
            return 1;
        }
    }
    close(sockfd);
    return 0;
}