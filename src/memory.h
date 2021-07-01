//
// Created by wzj on 2021/6/28.
//

#ifndef RISCV_SIMULATOR_MEMORY_H
#define RISCV_SIMULATOR_MEMORY_H
#include <vector>
#include <iomanip>
using std::vector;

class mem
{
private:
    const static int mem_size=500000;
    unsigned int memory[mem_size]{};
    unsigned int check[mem_size]{};
public:
    mem()
    {
        for(int i=0;i<mem_size;i++)
        {
            memory[i]=check[i]=0;
        }
    }
    void write(unsigned int beginPos, vector<unsigned int>memSetList)
    {
        for(int i=0;i<memSetList.size();i++)
        {
            memory[beginPos+i]=memSetList[i];
        }
    }
    void write(unsigned int pos,unsigned int memSetValue)
    {
        memory[pos]=memSetValue;
    }
    unsigned int read(unsigned int pos)
    {
        return memory[pos];
    }
    unsigned int& operator[](unsigned int pos)
    {
        return memory[pos];
    }
    void debug_print()
    {
        std::cerr<<"mem change"<<std::endl;
        for(int i=0;i<mem_size;i++)
        {
            if(check[i]!=memory[i])
            {
                std::cerr<<"at "<<std::setw(8)<<std::setfill(' ')<<std::hex<<i<<' '<<std::setw(8)<<std::setfill('0')<<check[i];
                std::cerr<<" => "<<std::setw(8)<<std::setfill('0')<<memory[i]<<std::hex<<std::endl;
                check[i]=memory[i];
            }
        }
    }
};


#endif //RISCV_SIMULATOR_MEMORY_H
