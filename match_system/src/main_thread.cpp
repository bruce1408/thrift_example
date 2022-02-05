// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "match_server/Match.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include<vector>
#include<iostream>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<queue>


using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::match_server;
using namespace std;

struct Task{

    User user;
    string type;
};

struct MessageQueue{
    queue<Task> q;
    mutex m;
    condition_variable cv;
}message_queue;

class Pool
{
    public:
    void save_result(int a, int b)
    {
        printf("match result %d, %d \n", a, b);
    }

    void match()
    {
        while(users.size() > 1)
        {
            auto a = users[0];
            auto b = users[1];
            users.erase(users.begin());
            users.erase(users.begin());
            save_result(a.id, b.id);
        }
    }

    void add(User user)
    {
        users.push_back(user);
    }

    void remove(User user)
    {
        for(size_t i =0; i<users.size(); i++)
        {
            if(users[i].id==user.id){
                users.erase(users.begin() + i);
                break;
            }
        }
    }
    private:
    vector<User> users;
}pool;


class MatchHandler : virtual public MatchIf {
    public:
        MatchHandler() {
            // Your initialization goes here
        }

        /**
         * user 添加的用户名
         * info 添加的信息
         * 在匹配池中添加一个用户
         * 
         * @param user
         * @param info
         */
        int32_t add_user(const User& user, const std::string& info) {
            // Your implementation goes here
            printf("add_user\n");
            // 只能有一个线程执行结束，才会执行第二个线程，然后会把锁释放掉
            unique_lock<mutex> lck(message_queue.m);
            message_queue.q.push({user, "add"});
            // 如果有新的任务，这里会进行唤醒，通知下面消费进行继续执行
            message_queue.cv.notify_all(); 
            return 0;
        }


        /**
         * user 删除的用户信息
         * info 附加信息
         * 从匹配池中删除一名用户
         * 
         * @param user
         * @param info
         */
        int32_t remove_user(const User& user, const std::string& info) {
            // Your implementation goes here
            printf("remove_user\n");

            // 不需要显式的进行解锁，执行结束会自动解锁
            unique_lock<mutex> lck(message_queue.m);
            message_queue.q.push({user, "remove"});
            message_queue.cv.notify_all(); 
            return 0;
        }
};


void consume_task()
{
    while (true)
    {
        unique_lock<mutex> lck(message_queue.m); 
        // 每次都从队列里面进行消费，如果是空就跳过
        if (message_queue.q.empty()){
            // 等待有任务进行唤醒
            message_queue.cv.wait(lck);
        }
        else {
            // task不是空，就拿出task，然后删除当前task
            auto task = message_queue.q.front();
            message_queue.q.pop(); 
            // 这里处理结束之后要进行解锁
            lck.unlock();
            if(task.type=="add") pool.add(task.user);
            else if( task.type=="remove") pool.remove(task.user); 

            pool.match();
        }
    }
}


int main(int argc, char **argv) {
    int port = 9090;
    ::std::shared_ptr<MatchHandler> handler(new MatchHandler());
    ::std::shared_ptr<TProcessor> processor(new MatchProcessor(handler));
    ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    ::std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    ::std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    cout<<"start my server ~"<<endl;
    TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

    thread matching_thread(consume_task);
    server.serve();
    return 0;
}

