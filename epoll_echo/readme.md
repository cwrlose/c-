***
前言：
推荐大家阅读一手 [详解epoll](https://blog.csdn.net/m0_73359068/article/details/146483073)
重点了解 epoll本质是在操作系统注册了一个事件表,然后将只需要查询"活跃文件操作表"
***
## 服务器 
无需多言,直接套用固定格式。
```cpp
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(8080);
    serv_addr.sin_family=AF_INET;

    errif(bind(sockfd,(sockaddr*)&serv_addr,sizeof(serv_addr))==-1,"socket bind error");
    errif(listen(sockfd,SOMAXCONN)==-1,"socket listen error");

```
说一下  bzero是初始化,将结构体化填充0字符，然后监听和绑定

```cpp
//epoll 部分
int epfd=epoll_createl(0);//事件表
struct epoll_event events[1024],ev;//等待队列
bzero(&ev,sizeof(ev));
ev.data.fd=sockfd;
ev.events=EPOLLIN|EPOLLET
setnoblock(fd);
epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,-1)//添加数组 简单的实现了一个可读和ET触发监控
```
ET为边缘触发，即是只报告一次！
直接部分简单的实现了 一个将sockfd添加到epoll维护的事件表上。
```cpp
//连接部分 
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

```
就如上面那样 先通过epoll_wait 中的等待队列 找出就绪队列，
然后开始"轮询"，检测到就绪队列中sockfd有"动作"（这个是tcp/ip），就进行连接，然后上同将客户端的sockfd添加进来。
```cpp
//业务部分
  else if (events[i].events&EPOLLIN) {
                while(true)
                {
                    //读取
                    char buf[1024];
                    bzero(&buf,sizeof(buf));

                    size_t read_bytes=read(events[i].data.fd,buf,sizeof(buf));
                    if(read_bytes>0) {
                        printf("Receive from client %d : %s\n",events[i].data.fd,buf);
                        write(events[i].data.fd,buf,sizeof(buf));
                    }
                    else if(read_bytes==-1&&errno == EINTR) {////客户端正常中断、继续读取
                        printf("continue\n");
                        continue;
                    }
                    else if(read_bytes==-1&& ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {//非阻塞IO，这个条件表示数据全部读取完毕
                        printf("finish reading once, errno: %d\n", errno);
                        break;
                    }
                    else if(read_bytes == 0){  //EOF，客户端断开连接
                        printf("EOF, client fd %d disconnected\n", events[i].data.fd);
                        close(events[i].data.fd);   //关闭socket会自动将文件描述符从epoll树上移除
                        break;
                    }
                }
            }

```
检测到可读事件 ,然后进行操作.echo 操作 去看一下base_echo部分

客户端没变


