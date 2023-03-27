#include "mprpconfig.h"
#include <iostream>
using namespace std;
void MprcpConfig::LoadConfigFile(const char *config_file)
{
  FILE *pf = fopen(config_file, "r");
  if (pf == nullptr)
  {
    cout << config_file << "is_invalid" << endl;
    exit(EXIT_FAILURE);
  }
  while (!feof(pf))
  {
    char buf[512] = {0};
    fgets(buf, 512, pf);
    string read_buf(buf);
    Trim(read_buf);
    if (read_buf[0] == '#' || read_buf.empty())
    {
      continue;
    }

    int idx = read_buf.find('=');
    if (idx == -1)
      continue;
    string key;
    string value;
    key = read_buf.substr(0, idx);
    Trim(key);
    int endidx=read_buf.find('\n',idx);   
    value =read_buf.substr(idx + 1, endidx  - idx-1  );
    Trim(key);  
    m_configMap.insert({key, value});
  }
}
string MprcpConfig::Load(const string &key)
{
  auto it = m_configMap.find(key);
  if (it == m_configMap.end())
  {
    return "";
  }
  return it->second;
}
void MprcpConfig::Trim(string &src_buf)
{
  int idx = src_buf.find_first_not_of(' ');
  if (idx != -1)
  {
    src_buf = src_buf.substr(idx, src_buf.size() - idx);
  }
  idx = src_buf.find_last_not_of(' ');
  if (idx != -1)
  {
    src_buf = src_buf.substr(0, idx + 1);
  }
}