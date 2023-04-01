#include<iostream>
#include<string>
#include"user.pb.h"
#include"mprpcapplication.h"
#include"rpcprovider.h"
#include"logger.h"
using namespace fixbug;
/*本地服务*/
class UserService:public UserServiceRpc
{
   public:
   bool Login(std::string name,std::string pwd)
   {
    std::cout<<"doing local service: login"<<std::endl;
    std::cout<<"name:"<<name<<"pwd:"<<pwd<<std::endl;
    return true;
   } 
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
         std::string name =request->name();
         std::string pwd =request->pwd();
         bool login_Result=Login(name,pwd);
         fixbug::ResultCode *code=response->mutable_result();
         code->set_errcode(0);
         code->set_errmsg("");
         response->set_success(login_Result);
         done->Run();

    }
};
int main(int argc,char**argv)
{
    // UserService us;
    // us.Login("lcy","123456");
    LOG_INFO("first log");
    LOG_ERR("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);
    MprpcApplication::Init(argc,argv);
    RpcProvider provider;
    provider.NotifyService(new UserService());
    provider.Run();
    return 0;
}