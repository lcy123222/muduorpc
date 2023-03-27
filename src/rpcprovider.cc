#include"rpcprovider.h"
#include<string>
#include"mprpcapplication.h"
#include<functional>
#include<google/protobuf/descriptor.h>
#include"rpcheader.pb.h"
void  RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    const google::protobuf::ServiceDescriptor *pserviceDesc=service->GetDescriptor();
    std::string service_name=pserviceDesc->name(); 
    int methodCnt=pserviceDesc->method_count();
    for(int i=0;i<methodCnt;i++)
    {
        const google::protobuf::MethodDescriptor* pmethodDesc=pserviceDesc->method(i);
        std::string method_name=pmethodDesc->name();
        service_info.m_methodMap.insert({method_name,pmethodDesc}); 
    }
}
   
void RpcProvider::Run()
{
    std::string ip=MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip,port); 
    //创建tcp对象
    muduo::net::TcpServer server(&m_eventloop,address,"RpcProvider");

    //绑定连接回调和消息读写回调 分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    //设置muduo库线程数量
    server.setThreadNum(4);
    std::cout<<"RcpPrpvider start service at ip: "<<ip<<"port: "<<port<<std::endl;
    //启动网络服务
    server.start();
    m_eventloop.loop();

}
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
      if(!conn->connected())
      {
        conn->shutdown();
      }
}
//框架内部 RpcProvider和RpcConsumer协商好之间通信用的protobuf 数据类型
//service_name method_name args  
void RpcProvider::OnMessage(const  muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer* buffer,muduo::Timestamp)
{
     std::string recv_buf = buffer->retrieveAllAsString();
     uint32_t header_size=0;
     recv_buf.copy((char*)&header_size,4,0);
     std::string rpc_header_Str=recv_buf.substr(4,header_size); 
     mprpc::RpcHeader rpcHeader;
     std::string service_name;
     std::string method_name;
     uint32_t args_size; 
    if(rpcHeader.ParseFromString(rpc_header_Str))
     {
          service_name=rpcHeader.service_name();
          method_name=rpcHeader.method_name();
          args_size=rpcHeader.args_size(); 
     }
     else
     {
        std::cout<<"rpc_header_Str:"<<rpc_header_Str<<"pares eeror"<<std::endl;
        return ;
     }
     std::string args_str=recv_buf.substr(4+header_size,args_size); 
     auto it=m_serviceMap.find(service_name);
     if(it== m_serviceMap.end())
     {
        std::cout<<service_name<<"is not exist"<<std::endl;
        return;
     }
    
     auto mit=it->second.m_methodMap.find(method_name);
     if(mit==it->second.m_methodMap.end())
     {
        std::cout<<service_name<<":"<<method_name<<"is not exist"<<std::endl;
        
     }
      google::protobuf::Service *service=it->second.m_service;
      const google::protobuf::MethodDescriptor* method=mit->second;
      google::protobuf::Message *request=service->GetRequestPrototype(method).New();
      if(!request->ParseFromString(args_str))
      {
        std::cout<<"request parse error,content:"<<args_str<<std::endl;
      }
      google::protobuf::Message *response=service->GetResponsePrototype(method).New();
      
      google::protobuf::Closure *done= google::protobuf::NewCallback<RpcProvider,
                                          const muduo::net::TcpConnectionPtr&,
                                         google::protobuf::Message*>
                                        (this,&RpcProvider::SendRpcResponse,conn,response);
      service->CallMethod(method,nullptr,request,response,done);
     
}
void  RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr&conn,google::protobuf::Message *response)
{
  std::string response_str;
  if(response->SerializeToString(&response_str))
  {
    conn->send(response_str);
    
  
  }
  else
  {
    std::cout<<"serialize response_str error!"<<std::endl;
  }
  conn->shutdown();
}