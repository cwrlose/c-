
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include<stdlib.h>
#define READ_BUFFER 1024
//错误处理
void errif(bool condition,const char* errmsg) {
    if(condition){
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}
//非阻塞 异步
void setnoblock(int fd) {
    fcntl(fd,F_SETFL,fcntl(fd,F_GETFD)|O_NONBLOCK);
}
void handleReadEvent(int sockfd) {
    char buf[READ_BUFFER];
    while(true){    //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if(bytes_read > 0){
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        } else if(bytes_read == -1 && errno == EINTR){  //客户端正常中断、继续读取
            printf("continue reading");
            continue;
        } else if(bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){//非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            break;
        } else if(bytes_read == 0){  //EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", sockfd);
            close(sockfd);   //关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    }


}
int main() {
    //绑定
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(8080);
    serv_addr.sin_family=AF_INET;

    errif(bind(sockfd,(sockaddr*)&serv_addr,sizeof(serv_addr))==-1,"socket bind error");
    errif(listen(sockfd,SOMAXCONN)==-1,"socket listen error");

    //epoll 初始化
    int epfd=epoll_create1(0);
    struct epoll_event events[1024],ev;
    bzero(&ev,sizeof(ev));
    ev.data.fd=sockfd;
    ev.events=EPOLLET|EPOLLIN;
    setnoblock(sockfd);//非阻塞
    epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&ev);//注册事件

    while(true) {
        int nfds=epoll_wait(epfd,events,1024,-1);//等待就绪数组
        errif(nfds==-1,"epoll wait error");
        //查询
        for(int i=0; i<nfds;i++) {
            //连接
            if(events[i].data.fd==sockfd) {
                struct sockaddr_in clnt_addr;
                bzero(&clnt_addr,sizeof(clnt_addr));
                socklen_t clnt_addr_len=sizeof(clnt_addr);
                int clnt_sockfd=accept(sockfd,(sockaddr*)&clnt_addr,&clnt_addr_len);
                errif(clnt_sockfd==-1,"socket accept error");
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

                //注册
                struct epoll_event ev;
                bzero(&ev,sizeof(ev));
                ev.data.fd=clnt_sockfd;
                ev.events=EPOLLIN|EPOLLET;
                setnoblock(clnt_sockfd);
                epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_sockfd,&ev);
            }
            else if (events[i].events&EPOLLIN) {
                handleReadEvent(events->data.fd);



            }
            else {
                printf("something else happened\n");
            }
        }
    }
    close(sockfd);
    close(epfd);
    return 0;
}


