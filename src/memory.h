//
// Created by wzj on 2021/6/28.
//

#ifndef RISCV_SIMULATOR_MEMORY_H
#define RISCV_SIMULATOR_MEMORY_H
#include <vector>
using std::vector;

class mem
{
private:
    const static int mem_size=500000;
    unsigned int memory[mem_size]{};
public:
    mem()
    {
        for(unsigned int & i : memory)
            i=0;
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
};


#endif //RISCV_SIMULATOR_MEMORY_H
