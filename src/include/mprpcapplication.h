#pragma once
#include"mprpconfig.h"
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