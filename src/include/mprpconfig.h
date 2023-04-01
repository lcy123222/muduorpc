#pragma once
#include<unordered_map>
using namespace std;
//rpcserverip rpcserverport zookeeperip zookeeperport
class MprcpConfig
{
    public:
    void LoadConfigFile(const char *config_file);
    string Load(const string &key);
    private:
    unordered_map<string,string> m_configMap;
    void Trim(string &src_buf);

};