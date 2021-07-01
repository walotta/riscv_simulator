//
// Created by wzj on 2021/6/29.
//

#ifndef RISCV_SIMULATOR_CORE_H
#define RISCV_SIMULATOR_CORE_H

#include "memory.h"
#include "register.h"
#include <iomanip>

class core
{
private:
    class environment
    {
    public:
        unsigned int cmd{};
        unsigned int imm{};
        unsigned int rd{};
        unsigned int rs1=0;
        unsigned int rs2=0;
        unsigned int shamt{};
        unsigned int funct3{},funct7{};
        unsigned int opcode;
        unsigned int wb_value=0;
        unsigned int mem_operator_address=0;
        char type{};
    };

    void expend_with_symbol(unsigned int& to_expend,int max_pos)
    {
        if(to_expend&((unsigned int)1<<max_pos))
        {
            for(int i=max_pos+1;i<32;i++)
                to_expend+=((unsigned int)1<<i);
        }
    }

    void IF(unsigned int& pc,environment& env)
    {
        env.cmd=0;
        for(int i=0;i<4;i++)
        {
            env.cmd<<=8;
            env.cmd+=mem_living[pc+3-i];
        }
    }

    void ID(unsigned int& pc,environment& env)
    {
        env.opcode=(unsigned int)0b01111111&env.cmd;
        switch(env.opcode)
        {
            case (unsigned int)0b0110111:
                env.type='U';
                break;
            case (unsigned int)0b0010111:
                env.type='U';
                break;
            case (unsigned int)0b1101111:
                env.type='J';
                break;
            case (unsigned int)0b1100111:
                env.type='I';
                break;
            case (unsigned int)0b1100011:
                env.type='B';
                break;
            case (unsigned int)0b0000011:
                env.type='I';
                break;
            case (unsigned int)0b0100011:
                env.type='S';
                break;
            case (unsigned int)0b0010011:
                env.type='I';
                break;
            case (unsigned int)0b0110011:
                env.type='R';
                break;
        }
        switch(env.type)
        {
            case 'R':
                env.rd=(env.cmd&((unsigned int)0b11111<<7))>>7;
                env.funct3=(env.cmd&((unsigned int)0b111<<12))>>12;
                env.rs1=(env.cmd&((unsigned int)0b11111<<15))>>15;
                env.rs2=(env.cmd&((unsigned int)0b11111<<20))>>20;
                env.funct7=(env.cmd&((unsigned int)0b1111111<<25))>>25;
                break;
            case 'I':
                env.rd=(env.cmd&((unsigned int)0b11111<<7))>>7;
                env.funct3=(env.cmd&((unsigned int)0b111<<12))>>12;
                env.rs1=(env.cmd&((unsigned int)0b11111<<15))>>15;
                env.imm=(env.cmd&((unsigned int)0b111111111111<<20))>>20;
                expend_with_symbol(env.imm,11);
                break;
            case 'S':
                env.funct3=(env.cmd&((unsigned int)0b111<<12))>>12;
                env.rs1=(env.cmd&((unsigned int)0b11111<<15))>>15;
                env.rs2=(env.cmd&((unsigned int)0b11111<<20))>>20;
                env.imm=0;
                env.imm+=((env.cmd&((unsigned int)0b11111<<7))>>7);
                env.imm+=((env.cmd&((unsigned int)0b1111111<<25))>>25)<<5;
                expend_with_symbol(env.imm,11);
                break;
            case 'B':
                env.funct3=(env.cmd&((unsigned int)0b111<<12))>>12;
                env.rs1=(env.cmd&((unsigned int)0b11111<<15))>>15;
                env.rs2=(env.cmd&((unsigned int)0b11111<<20))>>20;
                env.imm=0;
                env.imm+=((env.cmd&((unsigned int)0b1<<7))>>7)<<11;
                env.imm+=((env.cmd&((unsigned int)0b1111<<8))>>8)<<1;
                env.imm+=((env.cmd&((unsigned int)0b111111<<25))>>25)<<5;
                env.imm+=((env.cmd&((unsigned int)0b1<<31))>>31)<<12;
                expend_with_symbol(env.imm,12);
                break;
            case 'U':
                env.rd=(env.cmd&((unsigned int)0b11111<<7))>>7;
                env.imm=((env.cmd&((unsigned int)0b11111111111111111111<<12))>>12)<<12;
                expend_with_symbol(env.imm,31);
                break;
            case 'J':
                env.rd=(env.cmd&((unsigned int)0b11111<<7))>>7;
                env.imm=0;
                env.imm+=((env.cmd&((unsigned int)0b11111111<<12))>>12)<<12;
                env.imm+=((env.cmd&((unsigned int)0b1<<20))>>20)<<11;
                env.imm+=((env.cmd&((unsigned int)0b1111111111<<21))>>21)<<1;
                env.imm+=((env.cmd&((unsigned int)0b1<<31))>>31)<<20;
                expend_with_symbol(env.imm,20);
                break;
        }
        if(env.opcode==(unsigned int)0b0010011&&(env.funct3==0b001||env.funct3==0b101))
        {
            env.shamt=(env.cmd&((unsigned int)0b11111<<20))>>20;
            env.funct7=(env.cmd&((unsigned int)0b1111111<<25))>>25;
            env.imm=0;
        }
        env.rs1=reg_living.read(env.rs1);
        env.rs2=reg_living.read(env.rs2);
    }

    void EX(unsigned int& pc,environment& env)
    {
        switch(env.opcode)
        {
            case (unsigned int)0b0110111:
                //LUI
                env.wb_value=(env.imm);
                pc+=4;
                break;
            case (unsigned int)0b0010111:
                //AUIPC
                env.wb_value=(env.imm)+pc;
                pc+=4;
                break;
            case (unsigned int)0b1101111:
                //JAL
                env.wb_value=pc+4;
                pc+=env.imm;
                break;
            case (unsigned int)0b1100111:
                //JALR
                env.wb_value=pc+4;
                pc=(env.imm+env.rs1)&~1;
                break;
            case (unsigned int)0b1100011:
                switch(env.funct3)
                {
                    case 0b000:
                        //BEQ
                        if(env.rs1==env.rs2)
                            pc+=env.imm;
                        else
                            pc+=4;
                        break;
                    case 0b001:
                        //BNE
                        if(env.rs1!=env.rs2)
                            pc+=env.imm;
                        else
                            pc+=4;
                        break;
                    case 0b100:
                        //BLT
                        if((int)env.rs1<(int)env.rs2)
                            pc+=env.imm;
                        else
                            pc+=4;
                        break;
                    case 0b101:
                        //BGE
                        if((int)env.rs1>=(int)env.rs2)
                            pc+=env.imm;
                        else
                            pc+=4;
                        break;
                    case 0b110:
                        //BLTU
                        if(env.rs1<env.rs2)
                            pc+=env.imm;
                        else
                            pc+=4;
                        break;
                    case 0b111:
                        //BGEU
                        if(env.rs1>=env.rs2)
                            pc+=env.imm;
                        else
                            pc+=4;
                        break;
                }
                break;
            case (unsigned int)0b0000011:
                switch(env.funct3)
                {
                    case 0b000:
                        //LB
                        env.mem_operator_address=env.rs1+env.imm;
                        pc+=4;
                        break;
                    case 0b001:
                        //LH
                        env.mem_operator_address=env.rs1+env.imm;
                        pc+=4;
                        break;
                    case 0b010:
                        //LW
                        env.mem_operator_address=env.rs1+env.imm;
                        pc+=4;
                        break;
                    case 0b100:
                        //LBU
                        env.mem_operator_address=env.rs1+env.imm;
                        pc+=4;
                        break;
                    case 0b101:
                        //LHU
                        env.mem_operator_address=env.rs1+env.imm;
                        pc+=4;
                        break;
                }
                break;
            case (unsigned int)0b0100011:
                switch(env.funct3)
                {
                    case 0b000:
                        //SB
                        env.mem_operator_address=env.rs1+env.imm;
                        pc+=4;
                        break;
                    case 0b001:
                        //SH
                        env.mem_operator_address=env.rs1+env.imm;
                        pc+=4;
                        break;
                    case 0b010:
                        //SW
                        env.mem_operator_address=env.rs1+env.imm;
                        pc+=4;
                        break;
                }
                break;
            case (unsigned int)0b0010011:
                switch(env.funct3)
                {
                    case 0b000:
                        //ADDI
                        env.wb_value=env.rs1+env.imm;
                        pc+=4;
                        break;
                    case 0b001:
                        //SLLI
                        env.wb_value=(env.rs1)<<(env.shamt);
                        pc+=4;
                        break;
                    case 0b010:
                        //SLTI
                        env.wb_value=(unsigned int)((int)env.rs1<(int)env.imm);
                        pc+=4;
                        break;
                    case 0b011:
                        //SLTIU
                        env.wb_value=(unsigned int)(env.rs1<env.imm);
                        pc+=4;
                        break;
                    case 0b100:
                        //XORI
                        env.wb_value=env.rs1 xor env.rd;
                        pc+=4;
                        break;
                    case 0b101:
                        //SRLI||SRAI
                        switch(env.funct7)
                        {
                            case 0b0000000:
                                //SRLI
                                env.wb_value=env.rs1>>env.shamt;
                                pc+=4;
                                break;
                            case 0b0100000:
                                //SRAI
                                bool flag=false;
                                if(env.rs1&(1<<31))flag=true;
                                env.wb_value=env.rs1>>env.shamt;
                                if(flag)
                                {
                                    for(int i=0;i<env.shamt;i++)
                                    {
                                        env.wb_value+=(1<<(31-i));
                                    }
                                }
                                pc+=4;
                                break;
                        }
                        break;
                    case 0b110:
                        //ORI
                        env.wb_value=env.rs1|env.imm;
                        pc+=4;
                        break;
                    case 0b111:
                        //ANDI
                        env.wb_value=env.rs1&env.imm;
                        pc+=4;
                        break;
                }
                break;
            case (unsigned int)0b0110011:
                switch(env.funct3)
                {
                    case 0b000:
                        switch(env.funct7)
                        {
                            case 0b0000000:
                                //ADD
                                env.wb_value=env.rs1+env.rs2;
                                pc+=4;
                                break;
                            case 0b0100000:
                                //SUB
                                env.wb_value=env.rs1-env.rs2;
                                pc+=4;
                                break;
                        }
                        break;
                    case 0b001:
                        //SLL
                        env.wb_value=env.rs1<<(env.rs2&0b11111);
                        pc+=4;
                        break;
                    case 0b010:
                        //SLT
                        env.wb_value=((int)env.rs1<(int)env.rs2);
                        pc+=4;
                        break;
                    case 0b011:
                        //SLTU
                        env.wb_value=(env.rs1<env.rs2);
                        pc+=4;
                        break;
                    case 0b100:
                        //XOR
                        env.wb_value=env.rs1 xor env.rs2;
                        pc+=4;
                        break;
                    case 0b101:
                        switch(env.funct7)
                        {
                            case 0b0000000:
                                //SRL
                                env.wb_value=env.rs1>>(env.rs2&0b11111);
                                pc+=4;
                                break;
                            case 0b0100000:
                                //SRA
                                env.wb_value=env.rs1>>(env.rs2&0b11111);
                                if(env.rs1&(1<<31))
                                {
                                    for(int i=0;i<(env.rs2&0b11111);i++)
                                    {
                                        env.wb_value+=(1<<(31-i));
                                    }
                                }
                                pc+=4;
                                break;
                        }
                        break;
                    case 0b110:
                        //OR
                        env.wb_value=env.rs1|env.rs2;
                        pc+=4;
                        break;
                    case 0b111:
                        //AND
                        env.wb_value=env.rs1&env.rs2;
                        pc+=4;
                        break;
                }
                break;
        }
    }

    void MEM(unsigned int& pc,environment& env)
    {
        switch(env.opcode)
        {
            case (unsigned int)0b0000011:
                switch(env.funct3)
                {
                    case 0b000:
                        //LB
                        env.wb_value=mem_living.read(env.mem_operator_address);
                        expend_with_symbol(env.wb_value,7);
                        break;
                    case 0b001:
                        //LH
                        {
                            unsigned int tem;
                            tem=mem_living.read(env.mem_operator_address+1);
                            tem<<=8;
                            tem+=mem_living.read(env.mem_operator_address);
                            expend_with_symbol(tem,15);
                            env.wb_value=tem;
                        }
                        break;
                    case 0b010:
                        //LW
                        {
                            unsigned int tem=0;
                            for(int i=3;i>=0;i--)
                            {
                                tem<<=8;
                                tem+=mem_living.read(env.mem_operator_address+i);
                            }
                            env.wb_value=tem;
                        }
                        break;
                    case 0b100:
                        //LBU
                        env.wb_value=mem_living.read(env.mem_operator_address);
                        break;
                    case 0b101:
                        //LHU
                        unsigned int tem;
                        tem=mem_living.read(env.mem_operator_address+1);
                        tem<<8;
                        tem+=mem_living.read(env.mem_operator_address);
                        env.wb_value=tem;
                        break;
                }
                break;
            case (unsigned int)0b0100011:
                switch(env.funct3)
                {
                    case 0b000:
                        //SB
                        mem_living.write(env.mem_operator_address, env.rs2&0b11111111);
                        break;
                    case 0b001:
                        //SH
                        mem_living.write(env.mem_operator_address, env.rs2&0b11111111);
                        mem_living.write(env.mem_operator_address+1, ((env.rs2>>8)&(unsigned int)0b11111111));
                        break;
                    case 0b010:
                        //SW
                        for(int i=0;i<4;i++)
                        {
                            mem_living.write(env.mem_operator_address+i, (env.rs2>>(8*i))&((unsigned int)0b11111111));
                        }
                        break;
                }
                break;
        }
    }

    void WB(unsigned int& pc,environment& env)
    {
        switch(env.opcode)
        {
            case (unsigned int)0b0110111:
                //LUI
                reg_living.write(env.rd,env.wb_value);
                break;
            case (unsigned int)0b0010111:
                //AUIPC
                reg_living.write(env.rd,env.wb_value);
                break;
            case (unsigned int)0b1101111:
                //JAL
                reg_living.write(env.rd,env.wb_value);
                break;
            case (unsigned int)0b1100111:
                //JALR
                reg_living.write(env.rd,env.wb_value);
                break;
            case (unsigned int)0b0000011:
                switch(env.funct3)
                {
                    case 0b000:
                        //LB
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b001:
                        //LH
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b010:
                        //LW
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b100:
                        //LBU
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b101:
                        //LHU
                        reg_living.write(env.rd,env.wb_value);
                        break;
                }
                break;
            case (unsigned int)0b0010011:
                switch(env.funct3)
                {
                    case 0b000:
                        //ADDI
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b001:
                        //SLLI
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b010:
                        //SLTI
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b011:
                        //SLTIU
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b100:
                        //XORI
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b101:
                        //SRLI||SRAI
                        switch(env.funct7)
                        {
                            case 0b0000000:
                                //SRLI
                                reg_living.write(env.rd,env.wb_value);
                                break;
                            case 0b0100000:
                                //SRAI
                                reg_living.write(env.rd,env.wb_value);
                                break;
                        }
                        break;
                    case 0b110:
                        //ORI
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b111:
                        //ANDI
                        reg_living.write(env.rd,env.wb_value);
                        break;
                }
                break;
            case (unsigned int)0b0110011:
                switch(env.funct3)
                {
                    case 0b000:
                        switch(env.funct7)
                        {
                            case 0b0000000:
                                //ADD
                                reg_living.write(env.rd,env.wb_value);
                                break;
                            case 0b0100000:
                                //SUB
                                reg_living.write(env.rd,env.wb_value);
                                break;
                        }
                        break;
                    case 0b001:
                        //SLL
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b010:
                        //SLT
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b011:
                        //SLTU
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b100:
                        //XOR
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b101:
                        switch(env.funct7)
                        {
                            case 0b0000000:
                                //SRL
                                reg_living.write(env.rd,env.wb_value);
                                break;
                            case 0b0100000:
                                //SRA
                                reg_living.write(env.rd,env.wb_value);
                                break;
                        }
                        break;
                    case 0b110:
                        //OR
                        reg_living.write(env.rd,env.wb_value);
                        break;
                    case 0b111:
                        //AND
                        reg_living.write(env.rd,env.wb_value);
                        break;
                }
                break;
        }
    }

public:
    mem mem_living;
    reg reg_living;
    void run()
    {
        unsigned int pc=0;
        while(true)
        {
            environment env;
            //std::cerr<<"======\n@"<<std::hex<<std::setw(8)<<std::setfill('0')<<pc;
            if(pc==0x00001060)
                int iiiii=0;
            IF(pc,env);
            if(env.cmd==(unsigned int)0x0ff00513)
            {
                std::cout<<(((unsigned int)reg_living.read(10)) & 255u)<<std::endl;
                break;
            }
            ID(pc,env);
            EX(pc,env);
            MEM(pc,env);
            WB(pc,env);
            //std::cerr<<' '<<std::setw(8)<<std::setfill('0')<<env.cmd<<std::endl;
            //std::cerr<<"opcode ";
            //unsigned int tem=env.opcode;
            //int tem_times=7;
            //while((tem_times--)!=0){std::cerr<<tem%2?'1':'0';tem/=2;}
            //std::cerr<<" imm "<<std::dec<<(int)env.imm<<std::hex<<std::endl;
            //reg_living.debug_print();
            //mem_living.debug_print();
        }
    }
};

#endif //RISCV_SIMULATOR_CORE_H
