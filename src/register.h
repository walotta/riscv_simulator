//
// Created by wzj on 2021/6/28.
//

#ifndef RISCV_SIMULATOR_REGISTER_H
#define RISCV_SIMULATOR_REGISTER_H
#include <iomanip>

class reg
{
private:
    unsigned int regList[32]{};
    unsigned int debugList[32]{};
    std::string regName[32]{"x0","ra","sp","gp","tp","t0","t1","t2","s0","s1","a0","a1","a2","a3","a4","a5","a6","a7","s2","s3","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","t6"};
public:
    reg()
    {
        for(int i=0;i<32;i++)
        {
            regList[i]=debugList[i]=0;
        }
    }
    unsigned int read(unsigned int pos)
    {
        return regList[pos];
    }
    void write(unsigned int pos,unsigned int value)
    {
        if(pos!=0)regList[pos]=value;
    }
    void debug_print()
    {
        std::cerr<<"reg change"<<std::endl;
        for(int i=0;i<32;i++)
        {
            if(debugList[i]!=regList[i])
            {
                std::cerr<<"at "<<std::setw(3)<<std::setfill(' ')<<regName[i]<<' '<<std::dec<<std::setw(8)<<std::setfill('0')<<debugList[i];
                std::cerr<<" => "<<std::dec<<std::setw(8)<<std::setfill('0')<<regList[i]<<std::hex<<std::endl;
                debugList[i]=regList[i];
            }
        }
    }
};


#endif //RISCV_SIMULATOR_REGISTER_H
