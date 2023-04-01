#include"rpcprovider.h"
#include<string>
#include"mprpcapplication.h"
#include<functional>
#include<google/protobuf/descriptor.h>
#include"rpcheader.pb.h"
#include"zookeeperutil.h"
void  RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    const google::protobuf::ServiceDescriptor *pserviceDesc=service->GetDescriptor();
    std::string service_name=pserviceDesc->name(); 
    int methodCnt=pserviceDesc->method_count();
    cout<<"methodcnt: "<<methodCnt<<endl;
    LOG_INFO("servicename:%s",service_name);
    for(int i=0;i<methodCnt;i++)
    {
        const google::protobuf::MethodDescriptor* pmethodDesc=pserviceDesc->method(i);
        std::string method_name=pmethodDesc->name();
        service_info.m_methodMap.insert({method_name,pmethodDesc}); 
        LOG_INFO("method_name:%s",method_name);
    }
     service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
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
     // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    cout<<"mapsize"<<m_serviceMap.size()<<endl; // service_name为永久性节点    method_name为临时性节点
    for (auto &sp : m_serviceMap) 
    {
        // /service_name   /UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            // /service_name/method_name   /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    // rpc服务端准备启动，打印信息
    std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;
    //启动网络服务
    server.start();
    m_eventloop.loop();

}
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
      cout<<conn->name()<<endl;
      
      if(!conn->connected())
      {
        cout<<"onconnect false"<<endl;
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