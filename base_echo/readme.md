***
前言；
本教程 基于linux,需要读者有系统编程的基础
***
## 公共部分
异常处理远远大于你的程序代码 本身
```cpp
void errif(bool condition,const char* errmsg) {
    if(condition) {
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}
```
perror 为一个错误处理函数,输出到stdout
## 服务端
```cpp
//绑定socket部分
 int sockfd=socket(AF_INET,SOCK_STREAM,0);//自动选协议
    errif(sockfd==-1,"create socket error");

    struct sockaddr_in serv_addr;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(8080);

    errif(bind(sockfd,(sockaddr*)&serv_addr,sizeof(serv_addr)),"socket bind error");
    errif(listen(sockfd,SOMAXCONN),"socket listen error");
```
讲一下
宏 INADDR_ANY 为`0.0.0.0` 就是可以监听所有IP地址
宏 SOMAXCONN为系统建议最大连接数
```cpp
//连接部分
struct sockaddr_in clnt_addr;
socklen_t clnt_socklen=sizeof(clnt_addr);
int clnt_fd=accept(sockfd,(sockaddr*)&clnt_addr,&clnt_socklen);
    printf("new client fd %d! IP: %s Port: %d\n", clnt_fd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
```
这个无疑连接
**注意** accept为阻塞函数,即是要等待连接 要不然printf的打印部分出不来
```cpp
//业务部分
while(true) {
        char buf[READ_BUF];
        bzero(&buf,sizeof(buf));

        size_t read_bytes=read(clnt_fd,buf,sizeof(buf));

        if(read_bytes>0) {
            printf("message from %d : %s\n",clnt_fd,buf);
            write(clnt_fd,buf,sizeof(buf));
        }
        else if(read_bytes==0) {
          printf("client fd %d disconnected\n", clnt_fd);
          close(clnt_fd);
          break;
            
        }
        else if (read_bytes == -1)
        {
            close(clnt_fd);
            errif(true, "socket read error");
        }

    }
```
read和write是系统函数(linux系统 即是跨平台不强)
先读后写 关系连接

>
>read：
>返回值 > 0：表示成功读取的字节数。例如，返回值为 10，表示成功读取了 10 个字节。
返回值 = 0：表示已经到达文件末尾（EOF），没有更多数据可读。这通常发生在读取文件时，文件内容已经全部读完//对于一个socket而言就是关系连接达到末尾。
返回值 < 0：表示读取操作失败。此时，errno 会被设置为相应的错误码，可以
通过 write同理

错误码表格
以下是将这些错误码及其描述整理成的 Markdown 表格：

| 错误码       | 值 (具体值可能因系统而异) | 描述                           |
|:-------------|:--------------------------|:-------------------------------|
| `EINTR`      | 通常为 4                  | 写入操作被信号中断             |
| `EAGAIN`     | 通常为 11                 | 在非阻塞模式下，无法写入数据   |
| `EWOULDBLOCK`| 通常与 `EAGAIN` 相同      | 在非阻塞模式下，无法写入数据   |
| `EFAULT`     | 通常为 14                 | 缓冲区指针无效                 |
| `EBADF`      | 通常为 9                  | 文件描述符无效                 |
| `ENOSPC`     | 通常为 28                 | 磁盘空间不足                   |

## 客户端
```cpp
//连接部分
  int sockfd=socket(AF_INET,SOCK_STREAM,0);
    errif(sockfd==-1,"create socket error");

    struct sockaddr_in clnt_addr;
    clnt_addr.sin_family=AF_INET;
    clnt_addr.sin_port=htons(8080);
    clnt_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

    errif(connect(sockfd,(sockaddr*)&clnt_addr,sizeof(clnt_addr))==-1,"socket connect error");
```
客户端 没有所谓 bind和监听
```cpp
//业务部分
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
        }else if(read_bytes == -1){
            close(sockfd);
            errif(true, "socket read error");
        }
```
>scanf("%s",buf); 在客户端代码中，你使用了scanf("%s", buf)来读取用户输入。scanf函数的%s格式说明符会读取一个字符串，直到遇到空格或换行符为止。这意味着，如果用户输入的内容包含空格，scanf只会读取到第一个空格之前的字符串，剩下的部分会被丢弃。因此，客户端发送到服务器的数据不包含完整的用户输入内容。
> 使用fget fgets函数可以读取一行完整的输入，包括空格，直到遇到换行符为止。因此，你可以使用fgets代替scanf来读取用户输入。

** 运行命令**
>g++ server.cpp -o server
>g++ client.cpp -o client

>分别再不同的终端 ./ server . /client







