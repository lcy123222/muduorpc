# include<iostream>
#include"mprpcapplication.h"
#include"user.pb.h"

int main(int argc,char **argv)
{
    MprpcApplication::Init(argc,argv);
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    fixbug::LoginResponse response;
    MprpcController  controller;
    stub.Login(&controller,&request,&response,nullptr);
    if(controller.Failed())
    {
         std::cout<<controller.ErrorText()<<std::endl;
    }
    else
    {
          if(0==response.result().errcode())
    {
        std::cout<<"rpc login response success:"<<response.success()<<std::endl;
    }
    else
    {
        std::cout<<"rpc login response error:"<<response.result().errmsg()<<std::endl;
    }
    return 0;
    }
    
}