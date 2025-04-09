***
网络库解析
***
## Inetaddress
### 类成员
- 地址族 (为sock设计)
- 地址族的长度(为accept 设计)
### 构造函数
- 默认构造函数(为accept 设计)
- 赋值构造函数 (为sock 设计）:InetAddress(const char *ip, uint16_t port):addr_len(sizeof(sockaddr)
## 析构函数
- 为默认

## Socket

### 类成员
- int fd :返回值
### 构造函数
- 默认构造函数 :为socket()
- 赋值构造函数 :Socket::Socket(int _fd):fd(_fd):为accept设计
### 析构函数
- 将fd关闭 并且设置为-1;
### 类成员
- bind函数:void bind(InetAddress*);
- listen函数:lisen()
- setnoblock() 设为非阻塞
- accept(InetAddress*) 连接

## EPOLL
### 类成员
- int epfd 注册事件表
- struct epoll_event event:用一个 :堆的大小远远大于栈的大小
### 析构函数
- Epoll::Epoll():epfd(-1), event(nullptr) 将设为
  最后epfd利用epoll函数 event使用new函数
### 析构函数
这个为 删除event
### 类成员
- void addFd(int fd,uint32_t op); sockfd和监听类型
-  std::vector<epoll_event> poll(int timeout = -1); 设置超时 默认阻塞

## util
### 错误处理  errif(bool condition ,const char* errmsg)
### 业务处理  handleReadEvent(int sockfd)
## 整体修改思想
基于epoll_event中的ptr结构

ptr的好处
在 epoll 机制中，当一个文件描述符上有事件发生时，epoll_wait 会返回包含该事件信息的 epoll_event 结构体数组。但仅通过文件描述符本身，有时无法携带足够的上下文信息。
例如，在一个服务器程序中，每个客户端连接对应一个套接字文件描述符，除了文件描述符，你可能还需要记录客户端的其他信息，如客户端的地址、会话状态等。使用 ep.data.ptr 可以将一个指向自定义结构体的指针传递给 epoll，这样当事件发生时，就可以通过这个指针获取额外的上下文信息。
ep.data.ptr 允许你将任意类型的数据与文件描述符关联起来。你可以根据自己的需求定义不同的结构体来存储相关信息，然后将结构体的指针赋值给 ep.data.ptr。
这种灵活性使得你可以在不同的场景下使用不同的数据结构，而不需要局限于 epoll_event 结构体本身提供的信息。
在事件处理函数中，通过 ep.data.ptr 可以直接访问关联的额外数据，避免了在全局数据结构中查找与文件描述符对应的信息的过程，从而简化了事件处理的逻辑。
例如，在处理客户端连接关闭事件时，你可以直接通过 ep.data.ptr 访问客户端的会话信息，并进行相应的清理工作，而不需要通过文件描述符去查找对应的会话信息。

即一个ptr指向的数据结构为fd思想

## 库文件
### EPOLL文件
- void updataChannel(Channel*);即是更新和添加fd;
- std::vector<Channel *> Epoll::poll(int timeout) 从返回就绪数组 变成 返回绑定就绪文件符的(方便文件管理)

### Channel文件
###我们为每一个添加到epoll的文件描述符都添加了一个Channel，用户可以自由注册各种事件、很方便地根据不同事件类型设置不同回调函数

#### 类成员
- Epoll *ep;//事件表
- int fd;//监听的文件描述符
- uint32_t events;//events表示希望监听这个文件描述符的哪些事件
- uint32_t revents;//文件描述符正在发生的事件
- bool inEpoll;//当前的channel是否在
### 类函数

- Channel(Epoll *_ep, int _fd);
- ~Channel();

//功能
- void enableReading();
//属性
- int getFd();
- uint32_t getEvents();
- uint32_t getRevents();
- bool getInEpoll();
- void setInEpoll();
- void setRevents(uint32_t);

## server

注册事件 -> ep->addFd(serv_sock->Getfd(),EPOLLIN|EPOLLET);
变成了
文件操作符来操控的对象 即一个fd为一个channel对象
Channel* ser_channel=new Channel(ep,serv_sock->Getfd());
ser_channel->enableReading();

//逻辑部分
即连接无疑由活跃数组的成员变成了channel的fd;
业务部分:即事件注册无疑变成了channel的事件

顺利的将fd转化成channel;
