#include"mprpcchannel.h"
#include<string>
#include"rpcheader.pb.h"
#include"errno.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netinet/in.h>
#include"mprpcapplication.h"
#include"mprpccontroller.h"
#include"zookeeperutil.h"
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor *sd =method->service();
    std::string service_name=sd->name();
    std::string method_name=method->name();
    std::string args_str;
    uint32_t args_size=0;
    if(request->SerializeToString(&args_str))
    {
        args_size=args_str.size();


    }
    else
    {
        std::cout<<"serialize request error!"<<std::endl;
        return;
    }
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);
    uint32_t header_size=0;
    std::string rpc_header_str;
    if(rpcHeader.SerializeToString(&rpc_header_str))
    { 
        header_size=rpc_header_str.size();

    }
    else
    {
      
        controller->SetFailed("serialize rpcheader error");
        return ;
    }
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));
    send_rpc_str+=rpc_header_str;
    send_rpc_str+=args_str;
    int clintfd=socket(AF_INET,SOCK_STREAM,0);
    if(-1==clintfd)
    {
        std::cout<<"create socket error! erron:"<<errno<<std::endl;
        char errtext[512]={0};
        sprintf(errtext,"create socket error! erron:%d",errno);
         controller->SetFailed(errtext);
        return ;
    }
    // std::string ip=MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    ZkClient zkCli;
    zkCli.Start();
    //  /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;
    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str()); 
    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(port);
    server_addr.sin_addr.s_addr=inet_addr(ip.c_str());

    if(-1 == connect(clintfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        // cout<<sizeof(server_addr)<<endl;
        // cout<<connect(clintfd,(struct sockaddr*)&server_addr,sizeof(server_addr))<<endl;
    
        
        close(clintfd);
         char errtext[512]={0};
        sprintf(errtext,"connect error erron::%d",errno);
         controller->SetFailed(errtext);
        return ;
        
    }
    if(-1==send(clintfd,send_rpc_str.c_str(),send_rpc_str.size(),0))
    {
         
        
         close(clintfd);
          char errtext[512]={0};
        sprintf(errtext,"send error erron:%d",errno);
         controller->SetFailed(errtext);
        
         return;
    }
    char recv_buf[1024]={0};
    int recv_size=0;
    if(-1==(recv_size=recv(clintfd,recv_buf,1024,0)))
    {

         close(clintfd);
          char errtext[512]={0};
        sprintf(errtext,"recv error erron::%d",errno);
         controller->SetFailed(errtext);
        return ;
    }
    // std::string response_str(recv_buf,0,recv_size);
    //  if(!response->ParseFromString(response_str))
    if(!response->ParseFromArray(recv_buf,recv_size))
     {
        
         close(clintfd);

          char errtext[512]={0};
        sprintf(errtext,"parse error erron::%d",errno);
         controller->SetFailed(errtext);
        return ;
     } 
     close(clintfd);


}