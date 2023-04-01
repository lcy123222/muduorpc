#include<iostream>
#include<string>
#include"test.pb.h"
using namespace fixbug;
using namespace std;
// int main()
// { 
//     LoginRequest req;
//     req.set_name("lcy");
//     req.set_pwd("123456");
//     string send_str;
//     if(req.SerializeToString(&send_str))
//     {
//            cout<<send_str<<endl;
//     }
//     LoginRequest reqB;
//     if(reqB.ParseFromString(send_str))
//     {
//         cout<<reqB.name()<<endl;
//         cout<<req.pwd()<<endl;
//     }
//     return 0;
// }
int main()
{
//     LoginResponse  rsp;
//     ResultCode *rc=rsp.mutable_result();
//     rc->set_errcode(1);
//     rc->set_errmsg("登陆处理失败");

    GetFriendListsResponse rsp;
    ResultCode *rc=rsp.mutable_result();
    
    rc->set_errcode(0);
    User *user1=rsp.add_froemd_list();
    user1->set_name("lcy");
    user1->set_age(10);
    user1->set_sex(User::MAN);
     User *user2=rsp.add_froemd_list();
    user1->set_name("lfj");
    user1->set_age(15);
    user1->set_sex(User::WOMAN);
    cout<<rsp.froemd_list_size()<<endl;
    const User userlog=rsp.froemd_list(1);
    string send_str;
    rsp.SerializeToString(&send_str);
     cout<<send_str<<endl;
    
}