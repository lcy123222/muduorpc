#pragma once
#include"mprpconfig.h"
#include"mprpcchannel.h"
#include"mprpccontroller.h"
class MprpcApplication
{
    public:
    static void Init(int argc,char**argv);
    static MprpcApplication& GetInstance();
    static MprcpConfig& GetConfig();
    private:
    static MprcpConfig m_config;
    
    MprpcApplication(){}
     MprpcApplication(const  MprpcApplication&)=delete;
    MprpcApplication(MprpcApplication&&)=delete;

};