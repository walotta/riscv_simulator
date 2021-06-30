#include <iostream>
#include "core.h"

unsigned int hex_to_dec(const std::string& in)
{
    unsigned int tem[2];
    for(int i=0;i<2;i++)
    {
        if(!(in[i]>='0'&&in[i]<='9'))
            tem[i]=in[i]-'A'+10;
        else
            tem[i]=in[i]-'0';
    }
    return 16*tem[0]+tem[1];
}

int main()
{
    core CPU;

    std::string readIn;
    bool stop_flag=true;
    std::cin>>readIn;
    while(stop_flag)
    {
        if(readIn=="@@@")break;
        int tar_address=0;
        for(int i=1;i<=8;i++)
        {
            tar_address*=16;
            tar_address+=readIn[i]-'0';
        }
        vector<unsigned int>insert_list;
        std::cin>>readIn;
        while(readIn[0]!='@')
        {
            insert_list.push_back(hex_to_dec(readIn));
            if(!(std::cin>>readIn))
            {
                stop_flag=false;
                break;
            }
        }
        CPU.mem_living.write(tar_address,insert_list);
    }
    CPU.run();
    return 0;
}
