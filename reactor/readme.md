***
前言：reactor作为一种高性能的范式，值得我们学习
本次目标 实现一个基于的reactor 具备echo功能的服务器
***
## 核心组件
Reactor本身是靠一个事件驱动的框架,无疑引出一个类似于moduo的"EventLoop "以及boost.asio中的context而言，不断获取**就绪**事件，其本质无疑是io多用复路实现的。

下面就是核心功能：
```cpp
void EventLoop::loop(){
    while(!quit){
    std::vector<Channel*> chs;
        chs = ep->poll();
        for(auto it = chs.begin(); it != chs.end(); ++it){
            (*it)->handleEvent();
        }
    }
}
```
当事件检测到了，无疑我们要进行分发，这个所以我们也可以借助一个核心组件
Channel来分发
```cpp
class Channel
{
private:
    EventLoop *loop;
    int fd;
    uint32_t events;
    uint32_t revents;
    bool inEpoll;
    std::function<void()> callback;
public:
    Channel(EventLoop *_loop, int _fd);
    ~Channel();

    void enableReading();//设置检测什么事件

    int getFd();
    uint32_t getEvents();
    uint32_t getRevents();
    bool getInEpoll();
    void setInEpoll();

    // void setEvents(uint32_t);
    void setRevents(uint32_t);
    void setCallback(std::function<void()>);
};
```
不难发现,其实这个核心函数为std::function<void()> callback和void setCallback(std::function<void()>);，设置不同事件的回调函数
```cpp
void Channel::setCallback(std::function<void()> _cb){
    callback = _cb;
}
```
有过reactor的基础的人，一般都知道我们将业务分成连接和业务处理
没基础也没关系[可以阅读这个Reactor解析](https://blog.csdn.net/m0_73359068/article/details/147036846?spm=1001.2014.3001.5502) 你也可以直接看图
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/9dca24229f0a47439d731b61ddfea0be.png)
这就引出了Acceptor类 专门处理连接的

>- Acceptor类存在于事件驱动EventLoop类中，也就是Reactor模式的main-Reactor
>- 类中的socket fd就是服务器监听的socket fd，每一个Acceptor对应一个socket fd
>- 这个类也通过一个独有的Channel负责分发到epoll，该Channel的事件处理函数handleEvent()会调用Acceptor中的接受连接函数来新建一个TCP连接
```cpp
class Acceptor
{
private:
    EventLoop *loop;
    Socket *sock;//每一个Acceptor对应一个socket fd
    Channel *acceptChannel;//这个类也通过一个独有的Channel负责分发到epoll
public:
    Acceptor(EventLoop *_loop);
    ~Acceptor();
    void acceptConnection();
    std::function<void(Socket*)> newConnectionCallback;
    void setNewConnectionCallback(std::function<void(Socket*)>);
};
```
这就引出为什么 的核心组件其连接组件Acceptor(EventLoop *_loop);和  void acceptConnection();
```cpp
Acceptor::Acceptor(EventLoop *_loop) :loop(_loop){

    sock=new Socket();
    InetAddress* addr=new InetAddress("127.0.0.1",8080);
    sock->bind(addr);
    sock->listen();
    sock->setnonblocking();

    acceptChannel=new Channel(_loop,sock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);//注册回调函数 进行连接
    acceptChannel->setCallback(cb);
    acceptChannel->enableReading();//注册事件
    delete addr;

}

void Acceptor::acceptConnection(){
    InetAddress*clnt_addr=new InetAddress();
    Socket* clnt_sock=new Socket(sock->accept(clnt_addr));
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->getAddr().sin_addr), ntohs(clnt_addr->getAddr().sin_port));
    clnt_sock->setnonblocking();
    newConnectionCallback(sock);
    delete clnt_addr;
}
```
讲一下这个 newConnectionCallback(sock); 这个就是业务逻辑处理。这份代码的不完整之处就在于 他的目的核心是实现一个功能 echo回响 所以直接连接之后给了一个echo的业务逻辑。引出一个connect 即每一个连接都有一个业务处理方法
>Connection类，这个类也有以下几个特点：

> - 类存在于事件驱动EventLoop类中，也就是Reactor模式的main-Reactor
>- 类中的socket fd就是客户端的socket fd，每一个Connection对应一个socket fd
>- 每一个类的实例通过一个独有的Channel负责分发到epoll，该Channel的事件处理函数handleEvent()会调用Connection中的事件处理函数来响应客户端请求
```cpp
class Connection {
private:
    EventLoop *loop;
    Socket *sock;
    Channel *channel;
    std::function<void(Socket*)> deleteConnectionCallback;//
public:
    Connection(EventLoop *_loop, Socket *_sock);
    ~Connection();

    void echo(int sockfd);
    void setDeleteConnectionCallback(std::function<void(Socket*)>);

};
```
通过这个函数也可以看出一个echo就是一个connection的回调函数。然后我们再来看一下另一个重要的资源释放.设置一个主动释放的connection的回调函数。
```
void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> _cb){
    deleteConnectionCallback = _cb;
} 
```
也是注册函数

## Reactor-server
```cpp
class Server
{
private:
    EventLoop *loop;
    //ADD 连接逻辑
    Acceptor *acceptor;
    std::map<int, Connection*> connections;//一一对应

public:
    Server(EventLoop*);
    ~Server();

    void handleReadEvent(int);
    void newConnection(Socket *serv_sock);
    void deleteConnection(Socket *sock);
};
```
这个reactor无疑包含了三个组件 事件驱动，连接逻辑和业务逻辑
```cpp
Server::Server(EventLoop *_loop) : loop(_loop){
    acceptor = new Acceptor(loop);//完成所有的连接步骤 就没设置回调函数
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);//这个是每次动态
    acceptor->setNewConnectionCallback(cb);//

}
void Server::newConnection(Socket *sock){
    Connection *conn = new Connection(loop, sock);
    std::function<void(Socket*)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->setDeleteConnectionCallback(cb);
    connections[sock->getFd()] = conn;

}
void Server::deleteConnection(Socket * sock){
    Connection *conn = connections[sock->getFd()];
    connections.erase(sock->getFd());
    delete conn;
}
```
- 解析
  这个reactor-main是不是有三个疑问
- 为什么连接步骤完成了
- 他的业务函数也就是echo在哪
- 为什么用map来装饰Connection

首先我们来回顾一下整体流程，我们要明确一个点 一个socket的绑定IP和端口，不算连接逻辑，也就是说当你初始化的时候就应该弄好了。
真正的连接逻辑应该是设置连接回调函数的时候，例如
```cpp
Acceptor::Acceptor(EventLoop *_loop) :loop(_loop){

    sock=new Socket();
    InetAddress* addr=new InetAddress("127.0.0.1",8080);
    sock->bind(addr);
    sock->listen();
    sock->setnonblocking();

    acceptChannel=new Channel(_loop,sock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);//注册回调函数 
    acceptChannel->setCallback(cb);/
    acceptChannel->enableReading();//注册事件
    delete addr;

}
```
acceptChannel->setCallback(cb)这个就连接逻辑，当有客户端来的时候 他就是执行这个逻辑。第一个疑问解决
然后 acceptor->setNewConnectionCallback(cb);这个是不是设置了业务逻辑，因为前面讲过 这个没有线程池而且业务逻辑单一，所有将echo默认每一个连接的业务逻辑 所有直接再连接回调的时候设置了一个业务逻辑（你去看connect的构造函数 就会发现他直接绑定了echo做业务逻辑）第二疑问解决
```cpp
Connection::Connection(EventLoop *_loop, Socket *_sock):loop(_loop),sock(_sock),channel(nullptr){
    //属性
    channel=new Channel(loop,sock->getFd());//
    std::function<void()>cb=std::bind(&Connection::echo,this,sock->getFd());//业务处理函数;
    channel->setCallback(cb);
    channel->enableReading();
}
Connection::~Connection(){
    delete channel;
    delete sock;
}
```
也就是说 你可以从这下手 换一个业务逻辑 实现其他的业务
第三个疑问：
>对于断开TCP连接操作，也就是销毁一个Connection类的实例。由于Connection的生命周期由Server进行管理，所以也应该由Server来删除连接。如果在Connection业务中需要断开连接操作，也应该和之前一样使用回调函数来实现.使用map就是好删除管理








