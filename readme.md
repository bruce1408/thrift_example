# thrift_lesson
## 第一步 实现cpp 和 py 客户端服务端的通信
### 服务器端
在match_system/src 文件下面执行如下命令：thrift -r --gen cpp ../../match.thrift
然后生成对应的服务器代码，对其进行符合自己业务场景的改造

然后把文件名gen-cpp改成match_server，修改服务器cpp文件对应的引用路径即可

### 客户端
在game/src 目录下面执行 thrift -r --gen py ../../match.thrift
然后生成的文件名改成match_client, 在官网复制一段客户端的代码，然后修改引用文件的路径即可;

## 测试
### 测试1 
基本的py和cpp 文件之间的add user通信

g++ -c main.cpp match_server/*.cpp

g++ *.o -o main -lthrift -pthread