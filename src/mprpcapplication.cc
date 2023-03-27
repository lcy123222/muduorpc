#include "mprpcapplication.h"
#include<iostream>
#include <unistd.h>
#include<string>
MprcpConfig MprpcApplication::m_config;
void ShowArgsHelp()
{
    std::cout<<"format: command -i <configfile>"<<std::endl;
}
void MprpcApplication::Init(int argc, char **argv)
{
    if(argc<2)
    {
         ShowArgsHelp();
         exit(EXIT_FAILURE);
    }
    int c=0;
    std::string config_file;
    while(((c=getopt(argc,argv,"i:"))!=-1))
    {
          switch (c)
          {
          case 'i':
            /* code */
            config_file=optarg;
            break;
          case '?':
          std::cout<<"invalid args"<<std::endl;
          ShowArgsHelp();
          exit(EXIT_FAILURE);
          case ':':
           std::cout<<"need configfile"<<std::endl;
          ShowArgsHelp();
          exit(EXIT_FAILURE);
          
          default:
            break;
          }
    }
    cout<<config_file<<endl;
    m_config.LoadConfigFile(config_file.c_str());
    cout<<"rpcserverip:"<<m_config.Load("rpcserverip")<<endl;

}
MprpcApplication &MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}
MprcpConfig& MprpcApplication::GetConfig()
{
  return m_config;
}