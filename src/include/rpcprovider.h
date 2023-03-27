#pragma once
#include"google/protobuf/service.h"
#include<unordered_map> 
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>
#include<string>
#include"mprpcapplication.h"
#include<functional>
#include<google/protobuf/descriptor.h>

//提供网络发布rpc服务网络对象类
class RpcProvider
{
    public:
    void NotifyService(google::protobuf::Service *service);
   
    void Run();
    private:
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor* >m_methodMap; 
    };
    std::unordered_map<std::string,ServiceInfo> m_serviceMap;
    muduo::net::EventLoop m_eventloop;
    void OnConnection(const muduo::net::TcpConnectionPtr &conn);
    void OnMessage(const  muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer*,muduo::Timestamp);
    void  SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message *);


};