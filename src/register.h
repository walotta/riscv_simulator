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
public:
    reg()
    {
        for(int i=0;i<32;i++)
        {
            regList[i]=debugList[i]=0;
        }
    }
    unsigned int& operator[](unsigned int pos)
    {
        return regList[pos];
    }
    void debug_print()
    {
        std::cerr<<"reg change"<<std::endl;
        for(int i=0;i<32;i++)
        {
            if(debugList[i]!=regList[i])
            {
                std::cerr<<"at "<<std::dec<<std::setw(2)<<std::setfill('0')<<i<<' '<<std::setw(8)<<std::setfill('0')<<debugList[i];
                std::cerr<<" => "<<std::hex<<std::setw(8)<<std::setfill('0')<<regList[i]<<std::endl;
                debugList[i]=regList[i];
            }
        }
    }
};


#endif //RISCV_SIMULATOR_REGISTER_H
